
# Generate RSA private key
openssl genrsa -out nibaozhu.cn-key.pem

# Insert certificate info
openssl req -new -key nibaozhu.cn-key.pem -out nibaozhu.cn-key.pem.req

# Generate certificate
openssl x509 -req -in nibaozhu.cn-key.pem.req -signkey nibaozhu.cn-key.pem -out nibaozhu.cn.pem
