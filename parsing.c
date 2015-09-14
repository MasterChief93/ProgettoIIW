#include <stdio.h>
#include <stdlib.h>
#include <wurfl.h>
#include <device.h>
#include <string.h>

void wurfl_interrogation(const char *user_agent, char *result) {
	const char* root = "wurfl.xml";

	wurfl_t* wurfl = wurfl_init(root,NULL);

	device_t* device = wurfl_match(wurfl,user_agent);
	
	char** capabilities = device_capabilities(device,NULL);
	char** caps_ptr = capabilities;

	result[0] = '\0';	

	while(caps_ptr!=NULL && *caps_ptr!=NULL) {
		if (strcmp(*caps_ptr,"resolution_width") == 0 || strcmp(*caps_ptr,"resolution_height") == 0) {
			fprintf(stdout, "%s: %s,\n", *caps_ptr, *(caps_ptr + 1));
			result = strcat(result, *(caps_ptr + 1));
			result = strcat(result, " ");
		}
		caps_ptr+=2;
	}

	//This function has to use the User-Agent string in order to find the correct image quality factor
	//It should return a string or a float value
}
