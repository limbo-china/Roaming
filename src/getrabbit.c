#include "getrabbit.h"

void getFromRabbit(hashtable_t* rdtable)
{
    // char hostname[20];
    // int port;
    // char rabbitusername[20];
    // char rabbituserpasswd[20];
    // char exchangename[20];
    // char routingkey[20];
    // char exchangetype[20];

    // if (argc < 5) {
    //  fprintf(stderr, "Usage: receive_logs_direct host port exchange routingkeys...\n");
    //  return 1;
    // }

    FILE *f;
    if((f=fopen("jsondata.txt","w+"))==NULL)
        printf("cannot open file.\n");

    // if(!getRabbitCfg(hostname,&port,rabbitusername,rabbituserpasswd,
    //     exchangename,routingkey,exchangetype)){
    //     printf("get rabbit configuration failed.\n");
    //     return ;
    // }
    const char *hostname ="10.213.73.8";
    int port = 5672;
    const char *rabbitusername = "msg_usr";
    const char *rabbituserpasswd = "msg_passwd";
    const char *exchangename = "roamExChange";
    const char *routingkey = "roamKey";
    const char *exchangetype ="direct";
    // puts(hostname);
    // printf("%d\n",port );
    // puts(rabbitusername);
    // puts(rabbituserpasswd);
    // puts(exchangename);
    // puts(routingkey);
    // puts(exchangetype);
    // hostname = "10.213.73.8";
    // port = 5672;
    // exchangename = "roamExChange";

    int sockfd;
    int channelid = 1;
    amqp_connection_state_t conn;
    conn = amqp_new_connection();

    die_on_error(sockfd = amqp_open_socket(hostname, port), "Opening socket");
    amqp_set_sockfd(conn, sockfd);
    die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, rabbitusername, rabbituserpasswd), "Logging in");
    amqp_channel_open(conn, channelid);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");

    amqp_exchange_declare(conn, channelid, amqp_cstring_bytes(exchangename), amqp_cstring_bytes(exchangetype), 0, 0,
        amqp_empty_table);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring exchange");

    amqp_queue_declare_ok_t* r = amqp_queue_declare(conn, channelid, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);
    //amqp_queue_declare_ok_t *r = amqp_queue_declare(conn,channelid,amqp_cstring_bytes("123123fjkls"),0,0,0,1,amqp_empty_table);
    //int i;
    //for(i = 4;i < argc;i++)
    //{
    //routingkey = "roamKey";
    amqp_queue_bind(conn, channelid, amqp_bytes_malloc_dup(r->queue), amqp_cstring_bytes(exchangename),
        amqp_cstring_bytes(routingkey), amqp_empty_table);
    //}

    amqp_basic_qos(conn, channelid, 0, 1, 0);
    amqp_basic_consume(conn, channelid, amqp_bytes_malloc_dup(r->queue), amqp_empty_bytes, 0, 0, 0, amqp_empty_table);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming");

    {
        amqp_frame_t frame;
        int result;
        amqp_basic_deliver_t* d;
        amqp_basic_properties_t* p;
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

            d = (amqp_basic_deliver_t*)frame.payload.method.decoded;
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
            p = (amqp_basic_properties_t*)frame.payload.properties.decoded;
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
                for (i = 0; i < frame.payload.body_fragment.len; i++) {
                    fprintf(f,"%c",*((char*)frame.payload.body_fragment.bytes+i));
                    if (*((char*)frame.payload.body_fragment.bytes + i) == '.')
                        sleep_seconds++;
                }
                jsonStrParse((char*)frame.payload.body_fragment.bytes, frame.payload.body_fragment.len, rdtable);
                //fprintf(f,"\n------------------------------------------\n");
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

            //if (hashtable_count(rdtable) > 5000) {
                
            //    break;
            //}
        }
    }

    die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
    die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
    die_on_error(amqp_destroy_connection(conn), "Ending connection");
}

int getRabbitCfg(char *_host, int *_port, char *_user, char *_pwd,
    char *_exchgName, char* _key, char *_type){

    if (!getConfigValue(CONFIG_PATH, "rabbitHost", _host)){
        printf("get rabbit host failed.\n");
        return 0;
    }

    char _p[10];
    if (!getConfigValue(CONFIG_PATH, "rabbitPort", _p)){
        printf("get rabbit port failed.\n");
        return 0;
    }
    *_port = atoi(_p);

    if (!getConfigValue(CONFIG_PATH, "rabbitUserName", _user)){
        printf("get rabbit username failed.\n");
        return 0;
    }

    if (!getConfigValue(CONFIG_PATH, "rabbitUserPasswd", _pwd)){
        printf("get rabbit password failed.\n");
        return 0;
    }

    if (!getConfigValue(CONFIG_PATH, "exChangeName", _exchgName)){
        printf("get exchange name failed.\n");
        return 0;
    }

    if (!getConfigValue(CONFIG_PATH, "directKey", _key)){
        printf("get key failed.\n");
        return 0;
    }

    if (!getConfigValue(CONFIG_PATH, "exchangeType", _type)){
        printf("get exchange type failed.\n");
        return 0;
    }
    return 1;
}