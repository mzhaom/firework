#!/usr/bin/python3 -u

'''Certificate manager for IPSec'''

import argparse
import json
import os
import os.path
import subprocess


class Config:
    keystore = 'keystore'
    C = 'US'
    organization = 'strongSwan'
    ca_name = 'strongSwan CA'

    def __init__(self, **kwargs):
        self.__dict__.update(kwargs)

    def get_ca_display_name(self):
        return self.get_display_name(self.ca_name)

    def get_display_name(self, common_name):
        return "C=%s, O=%s, CN=%s" % (
            self.C, self.organization, common_name)
        

def run_pki_command(output_file, *args):
    with open(output_file, 'w') as fp:
        subprocess.check_call(['pki'] + list(args), stdout=fp)

def generate_key(fname):
    '''Generate RSA stored in the given PEM file'''
    run_pki_command(fname, '--gen', '--outform', 'pem')


def maybe_check_non_exists(args, *files):
    if not args.update:
        for fname in files:
            if os.path.isfile(fname):
                raise Exception(fname + ' exists already, please re-run with --update')

def init_ca(args, config):
    if not os.path.isdir(config.keystore):
        os.makedirs(config.keystore)
    ca_key = os.path.join(config.keystore, 'ca.key.pem')
    ca_cert = os.path.join(config.keystore, 'ca.cert.pem')
    maybe_check_non_exists(args, ca_key, ca_cert)
    generate_key(ca_key)
    run_pki_command(ca_cert, '--self', '--in',
                    ca_key, '--dn', config.get_ca_display_name(),
                    '--ca', '--outform', 'pem')
    print('Self signed CA cert is created as ' + ca_cert)


def issue_cert(config, private_key_file,
               cert_file, *args):
    ca_key = os.path.join(config.keystore, 'ca.key.pem')
    ca_cert = os.path.join(config.keystore, 'ca.cert.pem')
    with open(cert_file, 'w') as fp:
        pubkey_proc = subprocess.Popen(['pki', '--pub', '--in', private_key_file],
                                       stdout = subprocess.PIPE)
        cert_proc = subprocess.Popen(
            ['pki', '--issue', '--cacert', ca_cert,
             '--cakey', ca_key, '--outform', 'pem'] + list(args),
            stdin = pubkey_proc.stdout,
            stdout = fp)
        pubkey_proc.wait()
        cert_proc.wait()


def add_server(args, config):
    server_key = os.path.join(config.keystore, args.common_name + '.key.pem')
    server_cert = os.path.join(config.keystore, args.common_name + '.cert.pem')
    maybe_check_non_exists(args, server_key, server_cert)
    generate_key(server_key)
    issue_cert(config, server_key, server_cert,
               '--dn', config.get_display_name(args.common_name),
               '--san', args.common_name,
               '--flag', 'serverAuth',
               '--flag', 'ikeIntermediate')
    print('Generated cert for %s in %s' % (
        args.common_name,
        server_cert))


def export_p12(args, config):
    key_file = os.path.join(config.keystore, args.common_name + '.key.pem')
    p12_file = os.path.join(config.keystore, args.common_name + '.p12')
    cert_file = os.path.join(config.keystore, args.common_name + '.cert.pem')
    ca_cert = os.path.join(config.keystore, 'ca.cert.pem')
    subprocess.check_call(
        ['openssl', 'pkcs12', '-export',
         '-inkey', key_file,
         '-in', cert_file,
         '-name', args.common_name,
         # It doesn't seem necessary to include ca cert in p12. At
         # least for IOS client.

         # '-certfile', ca_cert,
         # '-caname', config.ca_name,
         '-out', p12_file])
    print('Generated pkcs 12 file in ' + p12_file)


def add_client(args, config):
    client_key = os.path.join(config.keystore, args.common_name + '.key.pem')
    client_cert = os.path.join(config.keystore, args.common_name + '.cert.pem')
    maybe_check_non_exists(args, client_key, client_cert)
    generate_key(client_key)
    issue_cert(config, client_key, client_cert,
               '--dn', config.get_display_name(args.common_name))
    print('Generated cert for %s in %s' % (
        args.common_name,
        client_cert))
    if args.p12:
        export_p12(args, config)


def main():
    parser = argparse.ArgumentParser(
        description = __doc__,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument('--config',
                        default='config.json',
                        help='Config file to load.')
    parser.add_argument('--update', action='store_true',
                    help='If set, overwrite the existing file')
    subparsers = parser.add_subparsers(dest='command', help='sub-command help')
    sp = subparsers.add_parser('init_ca',
                               help='Create the self signed root ca')
    
    sp = subparsers.add_parser('add_server',
                               help='Create a certificate for IPSec server.')
    sp.add_argument('--common-name', required=True,
                    help='Common name of the server, usually the DNS name')
    sp = subparsers.add_parser('add_client',
                               help='Create a certificate for the given client')
    sp.add_argument('--common-name', required=True,
                    help='Id of the user, usually this could be the email address')
    sp.add_argument('--p12', action='store_true',
                    help='If set, create PKCS#12 file for the generated key')
    sp = subparsers.add_parser('export_p12',
                               help='Export PKCS 12 file for the given client/server')
    sp.add_argument('--common-name', required=True,
                    help='Id of the certificate')
    args = parser.parse_args()
    if not args.command:
        raise Exception('Missing command')
    config = Config(**json.load(open(args.config, 'r')))
    func = globals()[args.command]
    func(args, config)


if __name__ == '__main__':
    main()
