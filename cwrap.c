#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <microhttpd.h>

#define PORT            8888

#define POSTBUFFERSIZE  5120
#define MAXINPUTSIZE    4096
#define MAXOUTPUTSIZE   4096

#define GET             0
#define POST            1

struct connection_info_struct {
  int connectiontype;
  char *outputstring;
  struct MHD_PostProcessor *postprocessor;
};

//const char *askpage = "<html><body>\
//                       Input: <br />\
//                       <form action=\"/\" method=\"post\">\
//                        <input name=\"input\" type=\"text\">\
//                        <input type=\"submit\" value=\"Send\"></form>\
//                       </body></html>";
//

const char *askpage = "Nothing here, please issue a POST request of type application/json";                       

const char *outputpage = "%s";

const char *errorpage =
  "{ error: 1 }";


int statusreport(char * output) {
	//REPLACE THIS FUNCTION WITH YOUR OWN! RETURNING A NON-ZERO CODE IN CASE OF SUCCESS	
  int ret;

  strcpy(output,"Status ok, I think");
   
  return(ret); 
}



int process(const char * input, char * output) {
	//REPLACE THIS FUNCTION WITH YOUR OWN! RETURNING A NON-ZERO CODE IN CASE OF SUCCESS	
	const char * i = input;
	char * p = output;
	while (*i) {
		*p = toupper(*i);
		p++;
		i++;		
	}	   
    return (p - output);
}


static int send_page(struct MHD_Connection *connection, const char *page, int returncode) {
  int ret;
  struct MHD_Response *response;


  response = MHD_create_response_from_buffer(strlen (page), (void *) page, MHD_RESPMEM_PERSISTENT);
  if (!response) return MHD_NO;

  ret = MHD_queue_response(connection, returncode, response);
  MHD_destroy_response (response);

  return ret;
}

static int iterate_post(void *coninfo_cls, enum MHD_ValueKind kind, const char *key, const char *filename, const char *content_type, const char *transfer_encoding, const char *data, uint64_t off, size_t size)	{

  struct connection_info_struct *con_info = coninfo_cls;

  if (strcmp(key, "input") == 0) {
      if ((size > 0) && (size <= MAXINPUTSIZE)) {

          char * outputstring;
          outputstring = malloc(MAXOUTPUTSIZE);
          if (!outputstring) return MHD_NO;
		  
		  int ret = process(data, outputstring);
		  		  
          if (ret > 0) {
            con_info->outputstring = outputstring;
          } else {
            con_info->outputstring = NULL;
          }
      } else {
        con_info->outputstring = NULL;
      }

      return MHD_NO;
    }

  return MHD_YES;
}

static void request_completed(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe) {
  struct connection_info_struct *con_info = *con_cls;

  if (NULL == con_info)
    return;

  if (con_info->connectiontype == POST) {
      //MHD_destroy_post_processor(con_info->postprocessor);
      if (con_info->outputstring) free(con_info->outputstring);
    }

  free(con_info);
  *con_cls = NULL;
}



static int answer_to_connection(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) {
  if (*con_cls == NULL) {
      struct connection_info_struct *con_info;

      con_info = malloc(sizeof (struct connection_info_struct));
      if (con_info == NULL) return MHD_NO;
      con_info->outputstring = NULL;

      if (strcmp(method, "POST") == 0) {

          //reenable for application/x-www-form-urlencoded
          
          /*
          con_info->postprocessor = MHD_create_post_processor(connection, POSTBUFFERSIZE, iterate_post, (void *) con_info);
          
          if (NULL == con_info->postprocessor) {
              free(con_info);
              return MHD_NO;
          }*/

          con_info->connectiontype = POST;
      } else {
        con_info->connectiontype = GET;
      }

      *con_cls = (void *) con_info;

      return MHD_YES;
  }


    if (strcmp(method, "GET") == 0) {
        if ((strcmp(url,"/status/") == 0) || (strcmp(url,"/status") == 0)) {
            char * output;
            output = malloc(MAXOUTPUTSIZE);
            int ret = statusreport(output);
            int ret2 = send_page(connection, output, MHD_HTTP_OK);
            free(output);
            return ret;
        } else {
            return send_page(connection, askpage, MHD_HTTP_OK);
        }
    } else if (strcmp(method,"POST") == 0) {
        struct connection_info_struct *con_info = *con_cls;
        if (*upload_data_size != 0) {

            //MHD_post_process(con_info->postprocessor, upload_data, *upload_data_size);

            printf("INPUT=[%s] (%d)",upload_data, (int) *upload_data_size);
            char * outputstring;
            outputstring = malloc(MAXOUTPUTSIZE);
            int ret = process(upload_data, outputstring);
            if (ret > 0) con_info->outputstring = outputstring;
            *upload_data_size = 0;
            return MHD_YES;
        } else if (con_info->outputstring != NULL) {          
            return send_page (connection, con_info->outputstring, MHD_HTTP_OK);
        }

    }
    return send_page(connection, errorpage, MHD_HTTP_BAD_REQUEST);
}



int main () {
  struct MHD_Daemon *daemon;

  daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, &answer_to_connection, NULL, MHD_OPTION_NOTIFY_COMPLETED, &request_completed, NULL, MHD_OPTION_END);
                             
  if (daemon == NULL) return 1;

  printf("Webservice started, press any key to exit\n");
  getchar();

  MHD_stop_daemon(daemon);
  return 0;
}
