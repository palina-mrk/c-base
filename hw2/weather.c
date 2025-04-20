# include <stdio.h>
# include <stdlib.h>
# include <curl/curl.h>
# include <cjson/cJSON.h>

# define TOWNLEN 20
# define OK      1
# define BAD     0


// copies "from" into "to", returns number of copied symbols
int copy(char* to, char* from);
// is the short str substring of long str?
int str_compare(char* long_str, char* short_str);
int town_validate(char* town, int maxlen);
void form_address(char* address, char* town);
// writes the information from web into the file
void web2file   (char* url, char* filename, long int* bufsize);
// prints the information about the weather to the file
void jsonfile2screen(char* filename, long int bufsize, char* town);

int main (int argc, char** argv) {
  char address[TOWNLEN * 3];
  char filename[] = "tmp.json";
  long int bufsize;

  if(argc != 2){
    printf("Error: one parameter is required: name of the town\n");
    abort();
  } 

  form_address(address, argv[1]);
  web2file(address, filename, &bufsize);
  jsonfile2screen(filename, bufsize, argv[1]);

  return 0;
}

int copy(char* to, char* from){
  int ans = 0;
  while((*to = *from) != '\0'){
    ++to;
    ++from;
    ++ans;
  }

  return ans;
}

int town_validate(char* town, int maxlen){
  int diff = 'A'- 'a';
  
  if( *town >= 'a' && *town <= 'z')
    *town += diff;
  else if ( *town < 'A' || *town > 'Z' )
    return BAD;

  while(*(++town) != '\0'){
    if(--maxlen == 0)
      return BAD;

    if( *town >= 'A' && *town <= 'Z' )
      *town -= diff;
    else if ( *town < 'a' || *town > 'z')
      return BAD;
  }
  
  return OK;
}

void form_address(char* address, char* town){
  static char prefix[]  = "https://wttr.in/";
  static char postfix[] = "?format=j1";
  char* tmp = address;

  if(town_validate(town, TOWNLEN) == BAD){
    printf("Error: %s is invalid name of town!\n", town);
    abort();
  }

  tmp += copy(tmp, prefix);
  tmp += copy(tmp, town);
  tmp += copy(tmp, postfix);
  return;
}

void web2file   (char* url, char* filename, long int* bufsize){
  CURL *curl;
  CURLcode res;
  FILE *fp;

  fp = fopen(filename, "w");
  if( fp == NULL){
    printf("File opening error\n");
    fclose(fp);
    abort();
  }
  
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    /* Perform the request, 'res' holds the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
  }

  *bufsize = ftell(fp) + 100;
  fclose(fp);
  return;
}

void jsonfile2screen(char* filename, long int bufsize, char* town){
  FILE*    fp;
  char* jbuff; 
  cJSON *json;
  cJSON *curr_cond_arr;
  cJSON *curr_cond_obj;
  cJSON *w_state_arr;
  cJSON *w_state_obj;
  cJSON *w_state_val;
  cJSON *wind_dir;
  cJSON *wind_speed;
  cJSON *temp;
  cJSON *n_area_arr;
  cJSON *n_area_obj;
  cJSON *region_arr;
  cJSON *region_obj;
  cJSON *n_ctr_arr;
  cJSON *n_ctr_obj;
  cJSON *town_val;
  cJSON *ctry_val;

  fp = fopen(filename, "r");
  if( fp == NULL){
    printf("File opening error\n");
    fclose(fp);
    abort();
  }
  jbuff = calloc(bufsize, sizeof(char));
  fread(jbuff, sizeof(char), bufsize, fp);
  if(jbuff == NULL){
    printf("Memory allocation error!\n");
    fclose(fp);
    abort();
  }
  fclose(fp);

  /* init json object */
  json = cJSON_Parse(jbuff);
  if(json == NULL){
    printf("JSON parsing error!\n");
    cJSON_Delete(json);
    abort();
  }

  /* to check the correction if town name */
  n_area_arr = cJSON_GetObjectItemCaseSensitive(json, "nearest_area");
  n_area_obj = cJSON_GetArrayItem(n_area_arr, 0);
  region_arr = cJSON_GetObjectItemCaseSensitive(n_area_obj, "areaName");
  region_obj = cJSON_GetArrayItem(region_arr, 0);
  town_val  = cJSON_GetObjectItemCaseSensitive(region_obj, "value");
  n_ctr_arr = cJSON_GetObjectItemCaseSensitive(n_area_obj, "country");
  n_ctr_obj = cJSON_GetArrayItem(n_ctr_arr, 0);
  ctry_val  = cJSON_GetObjectItemCaseSensitive(n_ctr_obj, "value");
  if(str_compare(town_val->valuestring, town) == BAD)
    printf("Warning: required town was not found. Name of the town may be incorrect\n");
  printf("City: %s, country: %s\n", town_val->valuestring, ctry_val->valuestring);

  /* to read the weather state */
  curr_cond_arr = cJSON_GetObjectItemCaseSensitive(json, "current_condition");
  curr_cond_obj = cJSON_GetArrayItem(curr_cond_arr, 0);
  w_state_arr = cJSON_GetObjectItemCaseSensitive(curr_cond_obj, "weatherDesc");
  w_state_obj = cJSON_GetArrayItem(w_state_arr, 0);
  w_state_val = cJSON_GetObjectItemCaseSensitive(w_state_obj, "value");
  wind_dir   = cJSON_GetObjectItemCaseSensitive(curr_cond_obj, "winddir16Point");
  wind_speed = cJSON_GetObjectItemCaseSensitive(curr_cond_obj, "windspeedKmph");
  temp = cJSON_GetObjectItemCaseSensitive(curr_cond_obj, "temp_C");

  printf("Weather state: %s\n", w_state_val->valuestring);
  printf("Wind direction: %s, wind strench: %s km/h\n"
          , wind_dir->valuestring, wind_speed->valuestring);
  printf("Temperature: %s C\n", temp->valuestring);

  cJSON_Delete(json);
  free(jbuff);
  return;
}

int str_compare(char* long_str, char* short_str){
  while(*short_str != '\0'){
    if(*long_str != *short_str)
      return BAD;
    ++long_str;
    ++short_str;
  }

  return OK;
}

