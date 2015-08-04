#include <stdio.h>
#include <stdlib.h>
#include <wurfl.h>
#include <device.h>
#include <string.h>

void wurfl_interrogation(const char *user_agent, char *result) {
	const char* root = "wurfl.xml";
	//const char* patches[] = NULL;

	wurfl_t* wurfl = wurfl_init(root,NULL);
	//const char* user_agent = "Mozilla/5.0 (iPhone; CPU iPhone OS 6_0 like Mac OS X) AppleWebKit/536.26 (KHTML, like Gecko) Version/6.0 Mobile/10A5376e Safari/8536.25";

	device_t* device = wurfl_match(wurfl,user_agent);
	
	char** capabilities = device_capabilities(device,NULL);
	char** caps_ptr = capabilities;
	// char *result;

	// result = malloc(128*sizeof(char));
	// if (result == NULL) {
	// 	perror("malloc");
	// 	return EXIT_FAILURE;
	// }
	result[0] = '\0';	

	while(caps_ptr!=NULL && *caps_ptr!=NULL) {
		if (strcmp(*caps_ptr,"resolution_width") == 0 || strcmp(*caps_ptr,"resolution_height") == 0) {
			fprintf(stdout, "%s: %s,\n", *caps_ptr, *(caps_ptr + 1));
			result = strcat(result, *(caps_ptr + 1));
			result = strcat(result, " ");
			//printf("result = %s\n",result);
			//fflush(stdout);
		}
		caps_ptr+=2;
	}

	//This function has to use the User-Agent string in order to find the correct image quality factor
	//It should return a string or a float value
}

// void get_request() {
// 	wurfl_interrogation();
// 	//Writing of HTTP Response
// }

// int main() {
// 	wurfl_interrogation();
// 	return 0;
// 	//Writing of HTTP Response
// }
