// Fill in  your WiFi networks SSID and password
#define SECRET_SSID "SSID"
#define SECRET_PASS "PASS"

// Fill in the hostname of your AWS IoT broker
#define SECRET_BROKER "AWS_URL"

// Fill in the boards public certificate
const char SECRET_CERTIFICATE[] = R"(
-----BEGIN CERTIFICATE-----
CSR_CERT
-----END CERTIFICATE-----
)";
