#!/bin/sh

tempfile=$(tempfile -p cert_)

while read LINE
do
  echo "$LINE" >> "$tempfile"
  [[ "${LINE}" == "-----END CERTIFICATE-----" ]] && break
done

openssl x509 -fingerprint -noout -in "$tempfile"

rm $tempfile

tempfile=$(tempfile -p cert_)

cat > "$tempfile"

openssl x509 -fingerprint -noout -in "$tempfile"

rm $tempfile
