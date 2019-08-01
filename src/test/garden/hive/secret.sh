
# Generate RSA private key
openssl genrsa -out nibaozhu.cn-key.pem

# Insert certificate info
openssl req -new -key nibaozhu.cn-key.pem -out nibaozhu.cn-req.pem

# Generate certificate
openssl x509 -req -in nibaozhu.cn-req.pem -signkey nibaozhu.cn-key.pem -out nibaozhu.cn.pem
