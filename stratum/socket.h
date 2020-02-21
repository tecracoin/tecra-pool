
#define YAAMP_SOCKET_BUFSIZE	(128*1024) //max 10x POW

struct YAAMP_SOCKET
{
	char ip[64];
	int port;

//	pthread_mutex_t mutex;
	int sock;

	int buflen;
	int buflen_bos;
	char buffer[YAAMP_SOCKET_BUFSIZE];
	
	int last_read;
	int total_read;
};

bool socket_connected(YAAMP_SOCKET *s);

void socket_real_ip(YAAMP_SOCKET *s);

YAAMP_SOCKET *socket_initialize(int sock);
void socket_close(YAAMP_SOCKET *s);

json_value *socket_nextjson(YAAMP_SOCKET *s, YAAMP_CLIENT *client=NULL);
void socket_nextjson2(json_value * json, YAAMP_SOCKET *s, YAAMP_CLIENT *client=NULL);
int socket_send(YAAMP_SOCKET *s, const char *format, ...);

json_value *socket_nextjson_bos(YAAMP_SOCKET *s, YAAMP_CLIENT *client = NULL);
void socket_nextjson_bos2(json_value * json, YAAMP_SOCKET *s, YAAMP_CLIENT *client = NULL);
char *socket_nextjson_bos3(YAAMP_SOCKET *s, YAAMP_CLIENT *client = NULL);
int socket_send(YAAMP_SOCKET *s, const char *format, ...);

int socket_send_mtp(YAAMP_SOCKET *s, const char *format, ...);
int socket_send_mtp2(YAAMP_CLIENT *client, const char *format, ...);

int socket_send_raw(YAAMP_SOCKET *s, const char *buffer, int size);
int socket_send_raw_mtp(YAAMP_SOCKET *s, json_value * obj);

static union {
	struct {
		char line[108];
	} v1;
	struct {
		uint8_t sig[12];
		uint8_t ver_cmd;
		uint8_t fam;
		uint16_t len;
		union {
			struct {  /* for TCP/UDP over IPv4, len = 12 */
				uint32_t src_addr;
				uint32_t dst_addr;
				uint16_t src_port;
				uint16_t dst_port;
			} ip4;
			struct {  /* for TCP/UDP over IPv6, len = 36 */
				uint8_t  src_addr[16];
				uint8_t  dst_addr[16];
				uint16_t src_port;
				uint16_t dst_port;
			} ip6;
			struct {  /* for AF_UNIX sockets, len = 216 */
				uint8_t src_addr[108];
				uint8_t dst_addr[108];
			} unx;
		} addr;
	} v2;
} hdr;
