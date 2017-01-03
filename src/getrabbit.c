#include "getrabbit.h"

void getFromRabbit()
{
    char hostname[20];
    int port;
    char rabbitusername[20];
    char rabbituserpasswd[20];
    char exchangename[20];
    char routingkey[20];
    char exchangetype[20];

    // if (argc < 5) {
    //  fprintf(stderr, "Usage: receive_logs_direct host port exchange routingkeys...\n");
    //  return 1;
    // }

    if(!getRabbitCfg(hostname,&port,rabbitusername,rabbituserpasswd,
        exchangename,routingkey,exchangetype)){
        log_error(sms_conn->log, "Get rabbit configuration failed.\n");
        log_error(mms_conn->log, "Get rabbit configuration failed.\n");
        return ;
    }

    //remove the CR at the end.
    hostname[strlen(hostname)-1]=0;
    rabbitusername[strlen(rabbitusername)-1]=0;
    rabbituserpasswd[strlen(rabbituserpasswd)-1]=0;
    exchangename[strlen(exchangename)-1]=0;
    routingkey[strlen(routingkey)-1]=0;
    exchangetype[strlen(exchangetype)-1]=0;

    // const char *hostname ="10.213.73.8";
    // int port = 5672;
    // const char *rabbitusername = "msg_usr";
    // const char *rabbituserpasswd = "msg_passwd";
    // const char *exchangename = "roamExChange";
    // const char *routingkey = "roamKey";
    // const char *exchangetype ="direct";

    int sockfd;
    int channelid = 1;
    amqp_connection_state_t conn;
    conn = amqp_new_connection();

    die_on_error(sockfd = amqp_open_socket((const char *)hostname, port), "Opening socket");
    amqp_set_sockfd(conn, sockfd);
    die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, (const char *)rabbitusername, (const char *)rabbituserpasswd), "Logging in");
    amqp_channel_open(conn, channelid);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");

    amqp_exchange_declare(conn, channelid, amqp_cstring_bytes((const char *)exchangename), amqp_cstring_bytes((const char *)exchangetype), 0, 0,
        amqp_empty_table);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring exchange");

    amqp_queue_declare_ok_t* r = amqp_queue_declare(conn, channelid, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);
    //amqp_queue_declare_ok_t *r = amqp_queue_declare(conn,channelid,amqp_cstring_bytes("123123fjkls"),0,0,0,1,amqp_empty_table);
    //int i;
    //for(i = 4;i < argc;i++)
    //{
    //routingkey = "roamKey";
    amqp_queue_bind(conn, channelid, amqp_bytes_malloc_dup(r->queue), amqp_cstring_bytes((const char *)exchangename),
        amqp_cstring_bytes((const char *)routingkey), amqp_empty_table);
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
                    //fprintf(f,"%c",*((char*)frame.payload.body_fragment.bytes+i));
                    if (*((char*)frame.payload.body_fragment.bytes + i) == '.')
                        sleep_seconds++;
                }
                jsonStrParse((char*)frame.payload.body_fragment.bytes, frame.payload.body_fragment.len);
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

        }
    }

    die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
    die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
    die_on_error(amqp_destroy_connection(conn), "Ending connection");
}

int getRabbitCfg(char *_host, int *_port, char *_user, char *_pwd,
    char *_exchgName, char* _key, char *_type){

    if (!getConfigValue(CONFIG_PATH, "rabbitHost", _host)){
        log_error(sms_conn->log, "Get rabbit host failed.\n");
        log_error(mms_conn->log, "Get rabbit host failed.\n");
        return 0;
    }

    char _p[10];
    if (!getConfigValue(CONFIG_PATH, "rabbitPort", _p)){
        log_error(sms_conn->log, "Get rabbit port failed.\n");
        log_error(mms_conn->log, "Get rabbit port failed.\n");
        return 0;
    }
    *_port = atoi(_p);

    if (!getConfigValue(CONFIG_PATH, "rabbitUserName", _user)){
        log_error(sms_conn->log, "Get rabbit username failed.\n");
        log_error(mms_conn->log, "Get rabbit port failed.\n");
        return 0;
    }

    if (!getConfigValue(CONFIG_PATH, "rabbitUserPasswd", _pwd)){
        log_error(sms_conn->log, "Get rabbit password failed.\n");
        log_error(mms_conn->log, "Get rabbit port failed.\n");
        return 0;
    }

    if (!getConfigValue(CONFIG_PATH, "exChangeName", _exchgName)){
        log_error(sms_conn->log, "Get exchange name failed.\n");
        log_error(mms_conn->log, "Get rabbit port failed.\n");
        return 0;
    }

    if (!getConfigValue(CONFIG_PATH, "directKey", _key)){
        log_error(sms_conn->log, "Get key failed.\n");
        log_error(mms_conn->log, "Get rabbit port failed.\n");
        return 0;
    }

    if (!getConfigValue(CONFIG_PATH, "exchangeType", _type)){
        log_error(sms_conn->log, "Get exchange type failed.\n");
        log_error(mms_conn->log, "Get rabbit port failed.\n");
        return 0;
    }
    return 1;
}