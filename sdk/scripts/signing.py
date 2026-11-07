#!/home/alexius/.local/bin/python2
import sys, os, base64, subprocess, getopt

def usage():
    print "signing adapted"

def strip_cert(certfile):
    data=open(certfile,'rb').read()
    # remove PEM armor/newlines
    b64 = ''
    inbody=False
    for line in data.splitlines():
        if 'BEGIN CERTIFICATE' in line:
            inbody=True
            continue
        if 'END CERTIFICATE' in line:
            inbody=False
            continue
        if inbody:
            b64 += line.strip()
    # decode/re-encode clean
    der = base64.b64decode(b64)
    return base64.b64encode(der)

def sign_file(key, datafile):
    # use openssl dgst -sha1 -sign
    p = subprocess.Popen(['openssl','dgst','-sha1','-sign',key,'-passin','pass:1111'], stdin=open(datafile,'rb'), stdout=subprocess.PIPE, stderr=sys.stderr)
    out,err = p.communicate()
    if p.returncode != 0:
        sys.exit(1)
    return base64.b64encode(out)

def main():
    opts,args = getopt.getopt(sys.argv[1:], "ho:a:n:m:p:k:d:c:w:", ["help","output=","app-path=","app-name=","crypto-path=","devkey-file=","devcert-file=","cacert-file=","passphrase="])
    outputFile=None; appPath=None; appName=None; cryptoPath=None; devKeyFile=None; devCertFile=None; caCertFile=None; passphrase='1111'
    for o,a in opts:
        if o in ('-h','--help'): usage(); sys.exit()
        elif o in ('-o','--output'): outputFile=a
        elif o in ('-a','--app-path'): appPath=a
        elif o in ('-n','--app-name'): appName=a
        elif o in ('-p','--crypto-path'): cryptoPath=a
        elif o in ('-k','--devkey-file'): devKeyFile=a
        elif o in ('-d','--devcert-file'): devCertFile=a
        elif o in ('-c','--cacert-file'): caCertFile=a
        elif o in ('-w','--passphrase'): passphrase=a
    if cryptoPath is not None:
        if not devKeyFile: devKeyFile=os.path.join(cryptoPath,'badaDevPriKey.pem')
        if not devCertFile: devCertFile=os.path.join(cryptoPath,'badaDev.cer')
        if not caCertFile: caCertFile=os.path.join(cryptoPath,'badaCA.cer')
    for f,t in [(appPath,"Application path"),(appName,"Application name"),(devKeyFile,"Private key file"),(devCertFile,"Development certificate file"),(caCertFile,"CA certificate file")]:
        if f is None:
            sys.stderr.write("%s not specified\n"%t); sys.exit(2)
    appFile=os.path.join(appPath,'Bin',appName+'.exe')
    htbFile=os.path.join(appPath,'Info',appName+'.htb')
    manifestFile=os.path.join(appPath,'Info','manifest.xml')
    try:
        packageSignature=sign_file(devKeyFile, appFile)  # note bali signs appFile+manifest, but sign_file only one file. Need concat.
        # sign appFile+manifest concatenated
        tmp='/tmp/scratchpad/_pkg_concat'
        with open(tmp,'wb') as out:
            out.write(open(appFile,'rb').read())
            out.write(open(manifestFile,'rb').read())
        packageSignature=sign_file(devKeyFile,tmp)
        appSignature=sign_file(devKeyFile,htbFile)
        devcert=strip_cert(devCertFile)
        cacert=strip_cert(caCertFile)
        sig='''<?xml version="1.0"?>
<Signature>
  <FileList>
    <File>/Bin/%s.exe</File>
    <File>/Info/manifest.xml</File>
  </FileList>
  <certificateChain>
    <certificate>
%s
    </certificate>
    <certificate>
%s
    </certificate>
  </certificateChain>
  <SignValue>
    <Package>
%s
    </Package>
    <AppSignature>
%s
    </AppSignature>
  </SignValue>
</Signature>
'''%(appName,devcert,cacert,packageSignature,appSignature)
        out=sys.stdout if not outputFile else open(outputFile,'w')
        out.write(sig)
        if outputFile: out.close()
    except Exception as e:
        sys.stderr.write("%s\n"%e); sys.exit(1)

if __name__=='__main__':
    main()
