#!/bin/sh

# The approach to certificate verification that I'm trying to use is described
# on slides 71-74 of
# https://docs.google.com/present/view?id=df9sn445_206ff3kn9gs&pli=1
#
# A more traditional approach is possible, easier, and boring.

uri=$1
domain=${uri#*://}
domain=${domain%%/*}

issuer_tempfile=$(tempfile -p cert_)
server_tempfile=$(tempfile -p cert_)

function save_temp_certs {
  while read LINE
  do
    echo "$LINE" >> "$issuer_tempfile"
    [[ "${LINE}" == "-----END CERTIFICATE-----" ]] && break
  done

  cat > "$server_tempfile"
}

function fingerprint {
  r=$(openssl x509 -fingerprint -md5 -noout -in "$1")
  echo ${r#*=}
}

save_temp_certs

issuer_fingerprint=$(fingerprint "$issuer_tempfile")
rm "$issuer_tempfile"

server_fingerprint=$(fingerprint "$server_tempfile")
rm "$server_tempfile"

cert_dir=/tmp/uzbl-ssl

mkdir -p "$cert_dir/$domain"

trusted_path="$cert_dir/$domain/trusted"

# if the certificate was previously trusted for this domain, we trust it now.
# TODO: check if the certificate is revoked or expired
[ -e "$trusted_path" ] && grep -q "$server_fingerprint" "$trusted_path" && exit 0

if [ -s "$trusted_path" ]
then
  # we've seen this domain before, but this certificate has not been trusted
  # TODO: check perspectives and PKI
  true
else
  # we've never seen this domain
  # TODO: test that cert matches origin
  # TODO: check PKI and perspectives
  true
fi

echo "uri data:text/html,<title>Untrusted SSL Certificate</title><p>The SSL certificate sent by $domain is not trusted. <p><code>echo $server_fingerprint >> $trusted_path</code><p><a href='$uri'>retry" > "$UZBL_FIFO"
