#include "client.h"
#include "jsontostruct.h"
#include "datahash.h"
#include "rabbit_test.h"
#include <amqp.h>
#include <amqp_framing.h>
#include <assert.h>
#include "utils.h"

const int tr2KeepAlive = 3;
const int waitReconn = 5;
int g_sockfd = 0;

int datanum = 0;

void* heartBeatDetect()
{
    alarmHandler();
    while (1)
        ;
    return NULL;
}
void alarmHandler()
{
    if (g_sockfd == 0) {
        signal(SIGALRM, alarmHandler);
        alarm(tr2KeepAlive);
        return;
    }

    //sendHBMsg(g_sockfd); //发送心跳包进行检测

    signal(SIGALRM, alarmHandler); //重新定时
    alarm(tr2KeepAlive);
}
void sendHBMsg(int _sock)
{
    HB_MsgContent hbmsg = constructHBMsg();

    int n;
    if ((n = send(_sock, &hbmsg, sizeof(hbmsg), 0)) < 0) {
        //
        /////reconnect
        printf("reconnecting to server ...\n");

        close(g_sockfd);
        int t = connectToServ();
        if (t != 0)
            g_sockfd = t;

        printf("reconnect succeed.\n");
    }
    printf("hb :%d\n", n);
}
void sendFRepMsg(int _sock)
{
    FRep_MsgContent frepmsg = constructFRepMsg();

    int n;
    if ((n = send(_sock, &frepmsg, sizeof(frepmsg), 0)) < 0) {
        printf("connection broken!waitting for reconnecting ...\n");
        sleep(waitReconn); // wait for reconnecting
        //
    }
    printf("frep: %d\n", n);
}
void sendFDataFinMsg(int _sock)
{
    FData_FinMsgContent fdatafinmsg = constructFDataFinMsg();

    int n;
    if ((n = send(_sock, &fdatafinmsg, sizeof(fdatafinmsg), 0)) < 0) {
        printf("connection broken!waitting for reconnecting ...\n");
        sleep(waitReconn); // wait for reconnecting
        //
    }
    printf("fdatafin: %d\n", n);
}
void sendRDataMsg(RData_MsgContent* rdata, int _sock)
{
    int n;
    datanum++;
    int i;
    printf("%d: ", datanum);
    printf("%u", rdata->length);
    printf("%u", rdata->type);
    printf("%u", rdata->roamprovince);
    printf("%u", ntohs(rdata->region));
    for (i = 0; i < 12; i++)
        printf("%c", *((u_char*)rdata->usernumber + i));
    printf("%u", ntohl(rdata->time));
    printf("%d", rdata->action);
    printf("\n");

    char _datamsg[RDATAMSG_LENGTH + 1];
    _datamsg[0] = rdata->length;
    _datamsg[1] = rdata->type;
    _datamsg[2] = rdata->roamprovince;
    strncpy(_datamsg + 3, (char*)&rdata->region, 2);
    strncpy(_datamsg + 5, rdata->usernumber, 12);
    strncpy(_datamsg + 17, (char*)&rdata->time, 4);
    _datamsg[21] = rdata->action;
    if ((n = send(_sock, _datamsg, RDATAMSG_LENGTH, 0)) < 0) {
        printf("connection broken!waitting for reconnecting ...\n");
        sleep(waitReconn); // wait for reconnecting
        //
    }
    printf("rdata: %d\n", n);
}
void sendAllRData(hashtable_t* h)
{
    int i;
    void* e;
    for (i = 0; i < h->tablelength; i++) {
        e = h->table[i];
        while (NULL != e) {
            sendRDataMsg((RData_MsgContent*)e, g_sockfd);
            //sleep(1);
            e = (void*)*(unsigned long*)(e + h->offset);
        }
    }
}
int connectToServ()
{

    char server_ip[20] = { 0 };
    int server_port = 0;
    struct sockaddr_in servaddr;

    if (!getSrvCfg(server_ip, &server_port)) {
        printf("get server configuration failed.\n");
        return 0;
    }

    int sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    servaddr = initSockAddr(server_ip, server_port);

    printf("connecting to server ...\n");
    Connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    printf("connect succeed.\n");

    return sockfd;
}
void* roamClient()
{
    int t = connectToServ();
    if (t != 0)
        g_sockfd = t;
    else
        return NULL;

    hashtable_t* rdtable = hashtable_create(1000, sizeof(RData_MsgContent), 0, 0, rd_free, rd_hash, rd_compare);

    //getjson();


    const char *hostname;
    int port;
    const char *exchange;
    const char *routingkey;
    const char *exchangetype = "direct";

    // if (argc < 5) {
    //  fprintf(stderr, "Usage: receive_logs_direct host port exchange routingkeys...\n");
    //  return 1;
    // }


    // FILE *f;
    // if((f=fopen("jsondata.txt","w+"))==NULL)
    //     printf("cannot open file.\n");



    hostname = "10.213.73.8";
    port = 5672;
    exchange = "roamExChange";

    int sockfd;
    int channelid = 1;
    amqp_connection_state_t conn;
    conn = amqp_new_connection();

    die_on_error(sockfd = amqp_open_socket(hostname, port), "Opening socket");
    amqp_set_sockfd(conn, sockfd);
    die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "msg_usr", "msg_passwd"),"Logging in");
    amqp_channel_open(conn, channelid);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");

    amqp_exchange_declare(conn,channelid,amqp_cstring_bytes(exchange),amqp_cstring_bytes(exchangetype),0,0,
                          amqp_empty_table);
    die_on_amqp_error(amqp_get_rpc_reply(conn),"Declaring exchange");

    amqp_queue_declare_ok_t *r = amqp_queue_declare(conn,channelid,amqp_empty_bytes,0,0,0,1,amqp_empty_table);
    //amqp_queue_declare_ok_t *r = amqp_queue_declare(conn,channelid,amqp_cstring_bytes("123123fjkls"),0,0,0,1,amqp_empty_table);
    //int i;
    //for(i = 4;i < argc;i++)
    //{
        routingkey = "roamKey";
        amqp_queue_bind(conn,channelid,amqp_bytes_malloc_dup(r->queue),amqp_cstring_bytes(exchange),
                        amqp_cstring_bytes(routingkey),amqp_empty_table);
    //}

    amqp_basic_qos(conn,channelid,0,1,0);
    amqp_basic_consume(conn,channelid,amqp_bytes_malloc_dup(r->queue),amqp_empty_bytes,0,0,0,amqp_empty_table);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming");

    {
        amqp_frame_t frame;
        int result;
        amqp_basic_deliver_t *d;
        amqp_basic_properties_t *p;
        size_t body_target;
        size_t body_received;

        while (1) {
            amqp_maybe_release_buffers(conn);
            result = amqp_simple_wait_frame(conn, &frame);
            //fprintf(f,"Result %d\n", result);
            if (result < 0)
                break;

            //fprintf(f,"Frame type %d, channel %d\n", frame.frame_type, frame.channel);
            if (frame.frame_type != AMQP_FRAME_METHOD)
                continue;

            //fprintf(f,"Method %s\n", amqp_method_name(frame.payload.method.id));
            if (frame.payload.method.id != AMQP_BASIC_DELIVER_METHOD)
                continue;

            d = (amqp_basic_deliver_t *) frame.payload.method.decoded;
            // fprintf(f,"Delivery %u, exchange %.*s routingkey %.*s\n",(unsigned) d->delivery_tag,
            //     (int) d->exchange.len, (char *) d->exchange.bytes,
            //     (int) d->routing_key.len, (char *) d->routing_key.bytes);

            result = amqp_simple_wait_frame(conn, &frame);
            if (result < 0)
                break;

            if (frame.frame_type != AMQP_FRAME_HEADER) {
                //fprintf(stderr, "Expected header!");
                abort();
            }
            p = (amqp_basic_properties_t *) frame.payload.properties.decoded;
            if (p->_flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
                //fprintf(f,"Content-type: %.*s\n",
                //(int) p->content_type.len, (char *) p->content_type.bytes);
            }

            body_target = frame.payload.properties.body_size;
            body_received = 0;

            int sleep_seconds = 0;
            while (body_received < body_target) {
                result = amqp_simple_wait_frame(conn, &frame);
                if (result < 0)
                    break;

                if (frame.frame_type != AMQP_FRAME_BODY) {
                    //fprintf(stderr, "Expected body!");
                    abort();
                }

                body_received += frame.payload.body_fragment.len;
                assert(body_received <= body_target);

                int i;
                for(i = 0; i<frame.payload.body_fragment.len; i++)
                {
                    //fprintf(f,"%c",*((char*)frame.payload.body_fragment.bytes+i));
                    if(*((char*)frame.payload.body_fragment.bytes+i) == '.')
                        sleep_seconds++;
                }
                jsonStrParse((char *)frame.payload.body_fragment.bytes, rdtable);
                //fprintf(f,"\n");

            }

            if (body_received != body_target) {
                /* Can only happen when amqp_simple_wait_frame returns <= 0 */
                /* We break here to close the connection */
                break;
            }
            /* do something */
            sleep(sleep_seconds);

            amqp_basic_ack(conn, channelid, d->delivery_tag, 0);

            if (hashtable_count(rdtable) > 300) {
                int n;
                FReq_MsgContent* freqmsg = (FReq_MsgContent*)malloc(sizeof(FReq_MsgContent));
                if ((n = recv(g_sockfd, freqmsg, sizeof(FReq_MsgContent), 0)) == 3) {
                    if (freqmsg->msg_length == 3 && freqmsg->msg_type == 1) {
                        printf("received request message\n");
                        sendFRepMsg(g_sockfd);
                        sendAllRData(rdtable);
                        sendFDataFinMsg(g_sockfd);
                    }
                }
                break;
            }
        }
    }

    die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
    die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
    die_on_error(amqp_destroy_connection(conn), "Ending connection");


    //jsonStrParse(jsontest, rdtable);


    // for (;;) {
    //     int n;
    //     FReq_MsgContent* freqmsg = (FReq_MsgContent*)malloc(sizeof(FReq_MsgContent));
    //     if ((n = recv(g_sockfd, freqmsg, sizeof(FReq_MsgContent), 0)) == 3) {
    //         if (freqmsg->msg_length == 3 && freqmsg->msg_type == 1) {
    //             printf("received request message\n");
    //             sendFRepMsg(g_sockfd);
    //             sendAllRData(rdtable);
    //             sendFDataFinMsg(g_sockfd);
    //         }
    //     }
    // }

    //while (1) {
    //sleep(1);
    //sendRDataMsg(g_sockfd);
    // RData_MsgContent* rdata1 = j2s(test_json1);
    // RData_MsgContent* rdata2 = j2s(test_json2);

    // int i;
    // for(i=0;i<sizeof(RData_MsgContent);i++)
    //     printf("%d",*((char *)rdata1+i) );
    // printf("\n");
    // if(!hashtable_search(rdtable,rdata1)){
    //     printf("insert a record!\n");
    //     hashtable_insert(rdtable,rdata1);

    // }
    // printf("hash count: %d\n",hashtable_count(rdtable));

    // if(!hashtable_search(rdtable,rdata2)){
    //     printf("insert a record\n");
    //     hashtable_insert(rdtable,rdata2);

    // }
    // printf("hash count: %d\n",hashtable_count(rdtable));
    // hashtable_trace(rdtable);
    //}
    return NULL;
}





