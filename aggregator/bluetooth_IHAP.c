#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <sys/socket.h>
#include <bluetooth/rfcomm.h>
#include <stdio.h>
#include <unistd.h>


char dest[18] = "00:21:7E:01:59:D2"; //IHAP
int port = 1;
uint8_t svc_uuid_int[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xab, 0xcd };
		
/***********************************************************************************
* search_bt_uuid_service														   *
* Uses the uuid number to find the correct service in the bluetooth server		   *
* 																				   *
* Arguments: void                                                                  *
* Return: void																	   *
*		  																		   *
************************************************************************************/
void search_bt_uuid_service(void)
{
	//char dest[18] = "70:FD:46:BA:DB:C1";//Galaxy
	
    uuid_t svc_uuid;
    int err;
    bdaddr_t target;
    sdp_list_t *response_list = NULL, *search_list, *attrid_list;
    sdp_session_t *session = 0;

	str2ba( dest, &target ); //IHAP

    // connect to the SDP server running on the remote machine
    session = sdp_connect( BDADDR_ANY, &target, SDP_RETRY_IF_BUSY );

    // specify the UUID of the application we're searching for
    sdp_uuid128_create( &svc_uuid, &svc_uuid_int );
    search_list = sdp_list_append( NULL, &svc_uuid );

    // specify that we want a list of all the matching applications' attributes
    uint32_t range = 0x0000ffff;
    attrid_list = sdp_list_append( NULL, &range );

    // get a list of service records that have UUID 0xabcd
    err = sdp_service_search_attr_req( session, search_list, 
            SDP_ATTR_REQ_RANGE, attrid_list, &response_list);
	sdp_list_t *r = response_list;

	printf("Status: %d\n", err);
    // go through each of the service records
    for (; r; r = r->next ) {
        sdp_record_t *rec = (sdp_record_t*) r->data;
        sdp_list_t *proto_list;
		printf("1 loop\n");
        // get a list of the protocol sequences
        if( sdp_get_access_protos( rec, &proto_list ) == 0 ) {
			sdp_list_t *p = proto_list;
			printf("1 if\n");
			// go through each protocol sequence
			for( ; p ; p = p->next ) {
				sdp_list_t *pds = (sdp_list_t*)p->data;
				printf("2 loop\n");
				// go through each protocol list of the protocol sequence
				for( ; pds ; pds = pds->next ) {
					printf("3 loop\n");
					// check the protocol attributes
					sdp_data_t *d = (sdp_data_t*)pds->data;
					int proto = 0;
					for( ; d; d = d->next ) {
						printf("4 loop\n");
						switch( d->dtd ) { 
							case SDP_UUID16:
							case SDP_UUID32:
							case SDP_UUID128:
								proto = sdp_uuid_to_proto( &d->val.uuid );
								break;
							case SDP_UINT8:
								if( proto == RFCOMM_UUID ) {
									printf("rfcomm channel: %d\n",d->val.int8);
									port = (uint8_t) d->val.int8;
								}
								break;
						}
					}
				}
				sdp_list_free( (sdp_list_t*)p->data, 0 );
			}
			sdp_list_free( proto_list, 0 );

		}

        printf("found service record 0x%x\n", rec->handle);
        sdp_record_free( rec );
    }

    sdp_close(session);
	
}

/***********************************************************************************
* send_message_bluetooth														   *
* Send message through bluetooth												   *
* 																				   *
* Arguments: char* message, int msg_length                                         *
* Return: int 1 (message sent sucessufuly), 0 (error sending the message		   *
*		  																		   *
************************************************************************************/
int send_message_bluetooth (char* message, int msg_length){	
	int  result, sock, client, bytes_read, bytes_sent;
	struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
	char buffer[1024] = { 0 };
	socklen_t opt = sizeof(rem_addr);

	// local bluetooth adapter
	loc_addr.rc_family = AF_BLUETOOTH;
	//loc_addr.rc_bdaddr = *BDADDR_ANY;

	loc_addr.rc_channel = (uint8_t) port;
	str2ba( dest, &loc_addr.rc_bdaddr );

	// allocate socket
	sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	printf("socket() returned %d\n", sock);

	// bind socket to port 3 of the first available
	//result = bind(sock, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
	//printf("bind() on channel %d returned %d\n", port, result);
	int count_attempts=0;
	int status = 0;
	do{
		status = connect(sock, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
		count_attempts++;
	}while(status || count_attempts >5);
    // send a message
    if( status == 0 ) {
        status = write(sock, message, msg_length);
		printf("Sent Message\n");
		return 1;
    }

    if( status < 0 ){
		printf("Status error: message not sent\n");
		return 0;
	}
	
	// read data from the client
    bytes_read = read(sock, buffer, sizeof(buffer));
    if( bytes_read > 0 ) {
        printf("received [%s]\n", buffer);
    }
	
	return 0;
}
