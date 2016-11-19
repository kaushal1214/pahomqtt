/*------------------------------------------------------------------------
* Program to connect to broker using Asynchronus method of MQTT
*------------------------------------------------------------------------*/

#include<stdio.h>
#include<MQTTAsync.h>
#include<string.h>
//Broker Address
#define ADDRESS "127.0.0.1"

//Client ID
#define CLIENTID "SaM1214"

//Clietn ID 2
#define CLIENTID1 "Sam"

//Topic to subscribe
#define TOPIC "Marvel"

//Retained
#define RETAINED 0

//Quality of Service
#define QOS 1

//PAYOAD
#define PAYLOAD "Ready.."

//Path for Serial Communication, mine is /dev/ttyUS0
#define PATH "/dev/ttyUSB0"

//Set the Flag for disconnect
int Flag=0;

//Set Subscribe 
int Subscribe=0;

//File Opening
FILE *handler;
static char ch='\0'; 

MQTTAsync pclient;

/*----------------------------------------------------------------------------- 
* This method will fire only at the success delivery of the message
*-----------------------------------------------------------------------------*/ 
void onPubSendSuccess(void * context, MQTTAsync_successData* res)
{ 

         printf("Message has been send!\n");
         MQTTAsync client=(MQTTAsync)context;
         MQTTAsync_disconnectOptions d_opts=MQTTAsync_disconnectOptions_initializer; 
         d_opts.onSuccess=NULL; 
         d_opts.context=client; 
         MQTTAsync_disconnect(client,&d_opts);
} 





/*---------------------------------------------------------------------
*OnPubConnect
*---------------------------------------------------------------------*/
void onPubConnect(void* context,MQTTAsync_successData *value)
{	
	printf("Pub connected\n");
	
	MQTTAsync client =(MQTTAsync)context;
        MQTTAsync_responseOptions opts= MQTTAsync_responseOptions_initializer;
	opts.onSuccess=NULL;
        opts.context=client;
	MQTTAsync_message mymsg= MQTTAsync_message_initializer;
        mymsg.payload=PAYLOAD; 
        mymsg.payloadlen=strlen(PAYLOAD);
        mymsg.qos=QOS; 
        mymsg.retained=RETAINED;
        //Publish to Broker 69         
	MQTTAsync_sendMessage(client,TOPIC,&mymsg,&opts);

}



/*-------------------------------------------------------------------
* The method is used to print the message received from the Broker
*-------------------------------------------------------------------*/
int msgarv(void *context, char * topic, int topiclen, MQTTAsync_message *msg)
{
					//strncpy(tbuf,topic,topiclen+1);
	char data[128]={'\0'};
	strncpy(data,msg->payload,msg->payloadlen);	//strrncpy(pbuf,msg->payload,msg->payloadlen+1);
	printf("%s\n",data);

	//File opened and written the very first character received
	handler=fopen(PATH,"r+");
	fputc(data[0],handler);
	
	if(data[0]=='B')
	{
		MQTTAsync_responseOptions ops=MQTTAsync_responseOptions_initializer;
		ops.onSuccess=NULL;
		ops.context=pclient;
		MQTTAsync_message MY_ms=MQTTAsync_message_initializer;
		char buffer[128]={"Superman"};
		fgets(buffer,sizeof(buffer),handler);
		MY_ms.payload=buffer;
		MY_ms.payloadlen=strlen(buffer);
		MY_ms.qos=QOS;
		MY_ms.retained=RETAINED;
		MQTTAsync_sendMessage(pclient,TOPIC,&MY_ms,&ops);	
	}
	
	fclose(handler);
	MQTTAsync_freeMessage(&msg);
	MQTTAsync_free(topic);
	//memset(data,'\0',);
	return 1;
}


/*--------------------------------------------------------------------
*The method will execute when client gets subscribed
*--------------------------------------------------------------------*/
void onSubscribe(void * context, MQTTAsync_successData * value)
{
	printf("Client Subscribed\n");
	Subscribe=1;
}


/*---------------------------------------------------------------------
*This method will be fired when client gets disconnected ftom the user
*---------------------------------------------------------------------*/
void onDisconnect(void *context, MQTTAsync_successData* res)
{
	printf("Client Disconnected from the Broker\n");
	Flag=0;
}


/*----------------------------------------------------------------------------
* The method will get fire when there is successfull connectiont to the broker
*------------------------------------------------------------------------------*/
void onConnect(void * context, MQTTAsync_successData* res)
{
	MQTTAsync client =(MQTTAsync)context;
	
	MQTTAsync_responseOptions res_opt=MQTTAsync_responseOptions_initializer;
	res_opt.onSuccess=onSubscribe;
	res_opt.context=client;

	MQTTAsync_subscribe(client,TOPIC,QOS,&res_opt);	
}




void main()
{
	MQTTAsync client;

	//Client, Address, clientID,MQTTCLIENT_PERSISTENCE_NONE,NULL
	MQTTAsync_create(&client,ADDRESS,CLIENTID,MQTTCLIENT_PERSISTENCE_NONE,NULL);
	MQTTAsync_create(&pclient,ADDRESS,CLIENTID1,MQTTCLIENT_PERSISTENCE_NONE,NULL);


	//Connection Options
	MQTTAsync_connectOptions conn_opt= MQTTAsync_connectOptions_initializer;
	conn_opt.keepAliveInterval=20;
	conn_opt.cleansession=1;
	conn_opt.onSuccess=onConnect;
	conn_opt.onFailure=NULL;
	conn_opt.context=client;
	//////////////////////////////////////////////
	
	MQTTAsync_connectOptions P_conn_opt=MQTTAsync_connectOptions_initializer;
	P_conn_opt.keepAliveInterval=20;
	P_conn_opt.cleansession=1;
	P_conn_opt.onSuccess=onPubConnect;
	P_conn_opt.onFailure=NULL;
	P_conn_opt.context=pclient;


	MQTTAsync_setCallbacks(client,NULL,NULL,msgarv,NULL);

	MQTTAsync_connect(client,&conn_opt);
	MQTTAsync_connect(pclient,&P_conn_opt);		
	while(1)
	{
		 pause();
	}
	//Sub disconnect	
	MQTTAsync_disconnectOptions d_opts=MQTTAsync_disconnectOptions_initializer;
	d_opts.onSuccess=onDisconnect;
	d_opts.context=client;
	MQTTAsync_disconnect(client,&d_opts);	

	//Pub disconnect
	MQTTAsync_disconnectOptions P_d_opts=MQTTAsync_disconnectOptions_initializer;
	P_d_opts.onSuccess=NULL;
	P_d_opts.context=pclient;
	MQTTAsync_disconnect(pclient,&P_d_opts);	

	sleep(1);
	MQTTAsync_destroy(&client);
	MQTTAsync_destroy(&pclient);
}
