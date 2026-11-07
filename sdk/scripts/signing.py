#!/home/alexius/.local/bin/python2
import sys, os, base64, subprocess, getopt, hashlib, tempfile

def usage():
    print "usage: signing.py --app-path DIR --app-name NAME --crypto-path DIR --output FILE"

def strip_cert(certfile):
    data = open(certfile, 'rb').read()
    b64 = ''
    inbody = False
    for line in data.splitlines():
        if 'BEGIN CERTIFICATE' in line:
            inbody = True
            continue
        if 'END CERTIFICATE' in line:
            inbody = False
            continue
        if inbody:
            b64 += line.strip()
    der = base64.b64decode(b64)
    return base64.b64encode(der), der

def raw_sign(key, passphrase, digest):
    """RSA PKCS#1 v1.5 over a raw SHA1 digest, without DigestInfo."""
    tf = tempfile.NamedTemporaryFile(delete=False)
    tf.write(digest)
    tf.close()
    of = tf.name + '.sig'
    cmd = ['openssl', 'pkeyutl', '-sign', '-in', tf.name, '-inkey', key, '-out', of]
    if passphrase:
        cmd += ['-passin', 'pass:' + passphrase]
    p = subprocess.Popen(cmd, stderr=subprocess.PIPE)
    _, err = p.communicate()
    if p.returncode != 0:
        os.unlink(tf.name)
        raise Exception(err)
    sig = open(of, 'rb').read()
    os.unlink(tf.name)
    os.unlink(of)
    return sig

def main():
    opts, args = getopt.getopt(sys.argv[1:], "ho:a:n:p:k:d:c:w:",
        ["help", "output=", "app-path=", "app-name=", "crypto-path=",
         "devkey-file=", "devcert-file=", "cacert-file=", "passphrase="])
    outputFile = None; appPath = None; appName = None; cryptoPath = None
    devKeyFile = None; devCertFile = None; caCertFile = None; passphrase = '1111'
    for o, a in opts:
        if o in ('-h', '--help'): usage(); sys.exit()
        elif o in ('-o', '--output'): outputFile = a
        elif o in ('-a', '--app-path'): appPath = a
        elif o in ('-n', '--app-name'): appName = a
        elif o in ('-p', '--crypto-path'): cryptoPath = a
        elif o in ('-k', '--devkey-file'): devKeyFile = a
        elif o in ('-d', '--devcert-file'): devCertFile = a
        elif o in ('-c', '--cacert-file'): caCertFile = a
        elif o in ('-w', '--passphrase'): passphrase = a
    if cryptoPath is not None:
        if not devKeyFile: devKeyFile = os.path.join(cryptoPath, 'badaDevPriKey.pem')
        if not devCertFile: devCertFile = os.path.join(cryptoPath, 'badaDev.cer')
        if not caCertFile: caCertFile = os.path.join(cryptoPath, 'badaCA.cer')
    for f, t in [(appPath, "Application path"), (appName, "Application name"),
                 (devKeyFile, "Private key file"), (devCertFile, "Development certificate file"),
                 (caCertFile, "CA certificate file")]:
        if f is None:
            sys.stderr.write("%s not specified\n" % t); sys.exit(2)
    if outputFile is None:
        outputFile = os.path.join(appPath, 'Signature.xml')

    appFile = os.path.join(appPath, 'Bin', appName + '.exe')
    htbFile = os.path.join(appPath, 'Info', appName + '.htb')
    manifestFile = os.path.join(appPath, 'Info', 'manifest.xml')

    packageDigest = hashlib.sha1(open(appFile, 'rb').read() + open(manifestFile, 'rb').read()).digest()
    appDigest = hashlib.sha1(open(htbFile, 'rb').read()).digest()
    packageSignature = raw_sign(devKeyFile, passphrase, packageDigest)
    appSignature = raw_sign(devKeyFile, passphrase, appDigest)

    devcert, devder = strip_cert(devCertFile)
    cacert, _ = strip_cert(caCertFile)

    open(os.path.join(appPath, 'Info', 'signerCert.sys'), 'wb').write(devder)
    open(os.path.join(appPath, 'Info', 'appSign.sys'), 'wb').write(base64.b64encode(appSignature))

    sig = '''<?xml version="1.0" encoding="utf-8"?>
<Signature>
  <FileList>
    <File>/bin/%s.exe</File>
    <File>/info/manifest.xml</File>
  </FileList>
  <certificateChain>
    <certificate>%s</certificate>
    <certificate>%s</certificate>
  </certificateChain>
  <SignValue>
    <Package>%s</Package>
    <AppSignature>%s</AppSignature>
  </SignValue>
</Signature>
''' % (appName, devcert, cacert,
       base64.b64encode(packageSignature), base64.b64encode(appSignature))
    old = os.path.join(appPath, 'signature.xml')
    if os.path.exists(old) and os.path.abspath(old) != os.path.abspath(outputFile):
        os.unlink(old)
    open(outputFile, 'w').write(sig)

if __name__ == '__main__':
    main()
