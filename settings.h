
/*		
tcp_frto          			NET_TCP_FRTO=92,  // 0 default, 1 basic f-rto, 2 sack enhanced frto
tcp_frto_response 			NET_TCP_FRTO_RESPONSE=125 // default 0, 2 aggressive response (ideal)
tcp_keepalive_intvl 		NET_IPV4_TCP_KEEPALIVE_INTVL=68 // default 75, seconds between probes
tcp_keepalive_probes        NET_IPV4_TCP_KEEPALIVE_PROBES=46 // default 9, number of probes..
tcp_keepalive_time 			NET_IPV4_TCP_KEEPALIVE_TIME=45, //default 7200, numb of seconds before keepalives are issued
tcp_retries1         		NET_IPV4_TCP_RETRIES1=47, //default(min) 3, numb of retransmits before network layer involvement
tcp_retries2         		NET_IPV4_TCP_RETRIES2=48, //default 15, numb of retransmits before giving up
tcp_wmem 					NET_TCP_WMEM=84,  // vector of 3 ints [min,default,max] default is 16k, max 
*/



#include <stdio.h>
#include <stdlib.h>
#include <sys/sysctl.h>
#include <linux/sysctl.h>
#include <errno.h>
#include <string.h>

#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */
#define RESET   "\033[0m"

extern int globalval[8];

char tcp_frto[] = {"tcp_frto"};
char tcp_frto_response[] = {"tcp_frto_response"};
char tcp_keepalive_intvl[] = {"tcp_keepalive_intvl"};
char tcp_keepalive_probes[] = {"tcp_keepalive_probes"};
char tcp_keepalive_time[] = {"tcp_keepalive_time"};
char tcp_retries1[] = {"tcp_retries1"};
char tcp_retries2[] = {"tcp_retries2"};
char tcp_wmem[] = {"tcp_wmem"}; 


void clrscr()
{
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}


int evaluateSettingID(char setting[]){


    int settingval;
    int comparelim = 30;
    if(strncmp(setting,tcp_frto,comparelim) == 0){
      settingval = 92;
    }
    else if(strncmp(setting,tcp_frto_response,comparelim) == 0){
      settingval = 125;
    }
    else if(strncmp(setting,tcp_keepalive_intvl,comparelim) == 0){
      settingval = 68;
    }
    else if(strncmp(setting,tcp_keepalive_time,comparelim) == 0){
      settingval = 45;
    }
    else if(strncmp(setting,tcp_keepalive_probes,comparelim) == 0){
      settingval = 46;
    }
    else if(strncmp(setting,tcp_retries1,comparelim) == 0){
      settingval = 47;
    }
    else if(strncmp(setting,tcp_retries2,comparelim) == 0){
      settingval = 48;
    }
    else if(strncmp(setting,tcp_wmem,comparelim) == 0){
      settingval = 84;

    }
    else{
      printf("Error - Invalid setting");
    }
    return settingval;
}

int * getSettingValue(char setting[]){
    int settingval = evaluateSettingID(setting);
    int name[] = {3,5,settingval}; 
    int namelen = 3;
    int val[8];  /* 4 would suffice */
    size_t len = sizeof(val);
    int i, error;

    //Query sysctl current value
    error = sysctl (name, namelen, (void *)val, &len,
            NULL /* newval */, 0 /* newlen */);

    memcpy(globalval,val,8);
    return globalval;
}

void printSettingValue(char setting[]){
  int * oldvalue = getSettingValue(setting);
  size_t len = sizeof(oldvalue); 
  printf("Current Value: ");
  int i;
  for (i = 0; i < len/(sizeof(int)); i++){
  printf("%i\t", oldvalue[i]);
  }
}
void changeSetting(char setting[],int value){

    int settingval = evaluateSettingID(setting);

	//Constructing parameters for sysctl()
    int name[] = {3,5,settingval}; 
    int namelen = 3;
    int oldval[8];  /* 4 would suffice */
    size_t len = sizeof(oldval);
    int i, error;

    printSettingValue(setting);
    int newval[1];
    int newlen = sizeof(newval);

    /* assign newval[0] */
    newval[0] = value;

    // wmem edge case
    if(settingval == 84){
    	int wmemval[3] = {value, value, value};
    	int wmemlen = sizeof(wmemval);
    	error = sysctl (name, namelen, NULL /* oldval */, 0 /* len */, wmemval, wmemlen);
    }
    else{
    	error = sysctl (name, namelen, NULL /* oldval */, 0 /* len */, newval, newlen);
    }
}
                



void restoreDefaults(){

    printf("\n AM I AWAKE???");
    printf("\n AM I AWAKE???");
    printf("\n AM I AWAKE???");
    printf("\n AM I AWAKE???");
    printf("\n AM I AWAKE???");
    printf("\n AM I AWAKE???");
	int error;
	int namelen = 3;
	int newval[1];
	int newlen = sizeof(newval);


	int name[] = {3,5,92}; //frto
	newval[0] = 2;
    error = sysctl (name, namelen, NULL /* oldval */, 0 /* len */, newval, newlen);	

	newval[0] = 0;
    name[3] = 125; //frto_response
    error = sysctl (name, namelen, NULL /* oldval */, 0 /* len */, newval, newlen);	

    newval[0] = 75;
    name[3] = 68; //keepalive_intvl
    error = sysctl (name, namelen, NULL /* oldval */, 0 /* len */, newval, newlen);	

    newval[0] = 7200;
    name[3] = 45; //keepalive_time
    error = sysctl (name, namelen, NULL /* oldval */, 0 /* len */, newval, newlen);	

    newval[0] = 9;
    name[3] = 46; //keepalive_probes
    error = sysctl (name, namelen, NULL /* oldval */, 0 /* len */, newval, newlen);	

    newval[0] = 3;
    name[3] = 47; //tcp_retries1
    error = sysctl (name, namelen, NULL /* oldval */, 0 /* len */, newval, newlen);	

    newval[0] = 15;
    name[3] = 48; //tcp_retries2
    error = sysctl (name, namelen, NULL /* oldval */, 0 /* len */, newval, newlen);	


    name[3] = 84; //tcp_wmem
    int wmemdefault[3] = {4096, 16384, 4194304};
    int wmemlen = sizeof(wmemdefault);
    error = sysctl (name, namelen, NULL /* oldval */, 0 /* len */, wmemdefault, wmemlen);
}




void displayMenu(){
    clrscr();

    printf(BOLDRED">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n" RESET);

    printf(BOLDYELLOW "SETTING \t\t\t DEFAULT \t CURRENT \t\t DESCRIPTION\n" RESET);
    int * current_frto = getSettingValue(tcp_frto);
    printf("tcp_frto \t\t\t 2 \t\t %d  \t\t\t 1 (basic f-rto) \t 2 (sack enhanced frto) \n", current_frto[0]);
    int * current_frto_response = getSettingValue(tcp_frto_response);
    printf("tcp_frto_response \t\t 0 \t\t %d    \t\t\t 1 (conservative response) \t 2 (aggresive response, undoes congestion algos) \n", current_frto_response[0]);
    int * current_keepalive_probes = getSettingValue(tcp_keepalive_probes);   
    printf("tcp_keepalive_probes \t\t 9 \t\t %d    \t\t\t number of keepalive probes \n", current_keepalive_probes[0]);
    int * current_keepalive_intvl = getSettingValue(tcp_keepalive_intvl);   
    printf("tcp_keepalive_intvl \t\t 75 \t\t %d    \t\t\t number of seconds between probes \n", current_keepalive_intvl[0]);
    int * current_keepalive_time = getSettingValue(tcp_keepalive_time);   
    printf("tcp_keepalive_time \t\t 7200 \t\t %d    \t\t number of seconds before keepalives are issued) \n", current_keepalive_time[0]);
    int * current_retries1 = getSettingValue(tcp_retries1);
    printf("tcp_retries1 \t\t\t 3 \t\t %d    \t\t\t number of retransmits before network layer is involved \n", current_retries1[0]);
    int * current_retries2 = getSettingValue(tcp_retries2);
    printf("tcp_retries2 \t\t\t 15 \t\t %d    \t\t\t number of retransmits to send before giving up\n", current_retries2[0]);
    int * current_wmem = getSettingValue(tcp_wmem);
    printf("tcp_wmem \t\t\t 16K \t\t %d %d %d   \t used to regulate send buffer sizes \n", current_wmem[0], current_wmem[1], current_wmem[2]);


    printf(BOLDRED"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n"RESET);


    printf(BOLDYELLOW"\n-----------------COMMANDS-----------------\n"RESET);
    printf(BOLDWHITE"\n!restore"RESET) ;
    printf(" \t restore default settings");
    printf(BOLDWHITE"\n!setting"RESET) ;
    printf("  \t change a setting");
    printf(BOLDWHITE"\n!run"RESET) ;
    printf("  \t run server with current settings");
    printf("\n\nChoose an option: ");
}

void menu(){

    displayMenu();

    char optres[] = {"!restore"};
    char optset[] = {"!setting"};
    char optrun[] = {"!run"};

    bool changingSettings = true; 
    while(changingSettings){


        char userOption[20];
        scanf("%s", userOption);


        int comparelim = 30;
        if(strncmp(userOption,optres,comparelim) == 0){
            restoreDefaults();
            displayMenu();
        }
        if(strncmp(userOption,optset,comparelim) == 0){
            printf("\nChoose a setting to change: ");
            char userset[20];
            scanf("%s", userset);
            printSettingValue(userset);

            printf("\nAssign a new value to %s: ",userset);
            int userval;
            scanf("%d", &userval);

            changeSetting(userset,userval);
            displayMenu();
        }
        if(strncmp(userOption,optrun,comparelim) == 0){
            changingSettings = false;
            printf("\n");
        }
    }

}







 



