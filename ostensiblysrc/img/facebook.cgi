#! /bin/sh
ACCESS_TOKEN=$(cat /secrets/sean.anasta.si/facebook.access_token)
curl -kL "https://graph.facebook.com/v2.4/1229040183/picture?access_token=$ACCESS_TOKEN&type=large"

