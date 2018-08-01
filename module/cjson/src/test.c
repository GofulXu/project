#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "parse.h"
#define LOGIN_STATUS 1

char buf[1024*1024] = "\"streams\": [{\"index\": 0,\"codec_name\": \"h264\",\"codec_long_name\": \"H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10\",\"profile\": \"High\",\"codec_type\": \"video\",\"codec_time_base\": \"1/46\",\"codec_tag_string\": \"avc1\",\"codec_tag\": \"0x31637661\",\"width\": 768,\"height\": 432,\"coded_width\": 768,\"coded_height\": 432,\"has_b_frames\": 2,\"sample_aspect_ratio\": \"0:1\",\"display_aspect_ratio\": \"0:1\",\"pix_fmt\": \"yuv420p\",\"level\": 30,\"chroma_location\": \"left\",\"refs\": 1,\"is_avc\": \"true\",\"nal_length_size\": \"4\",\"r_frame_rate\": \"23/1\",\"avg_frame_rate\": \"23/1\",\"time_base\": \"1/23000\",\"start_pts\": 2000,\"start_time\": \"0.086957\",\"duration_ts\": 5152000,\"duration\": \"224.000000\",\"bit_rate\": \"679759\",\"bits_per_raw_sample\": \"8\",\"nb_frames\": \"5152\",\"disposition\": {    \"default\": 1,    \"dub\": 0,    \"original\": 0,    \"comment\": 0,    \"lyrics\": 0,    \"karaoke\": 0,    \"forced\": 0,\"hearing_impaired\": 0,\"visual_impaired\": 0,\"clean_effects\": 0,\"attached_pic\": 0},\
\"tags\": {
    \"creation_time\": \"2011-11-20 18:51:08\",
    \"language\": \"chi\",
    \"handler_name\": \"GPAC ISO Video Handler\"
}\
        },\
        {\
            \"index\": 1,
            \"codec_name\": \"aac\",
            \"codec_long_name\": \"AAC (Advanced Audio Coding)\",
            \"profile\": \"HE-AAC\",
            \"codec_type\": \"audio\",
            \"codec_time_base\": \"1/44100\",
            \"codec_tag_string\": \"mp4a\",
            \"codec_tag\": \"0x6134706d\",
            \"sample_fmt\": \"fltp\",
            \"sample_rate\": \"44100\",
            \"channels\": 2,
            \"channel_layout\": \"stereo\",
            \"bits_per_sample\": 0,
            \"r_frame_rate\": \"0/0\",
            \"avg_frame_rate\": \"0/0\",
            \"time_base\": \"1/22050\",
            \"start_pts\": 0,
            \"start_time\": \"0.000000\",
            \"duration_ts\": 4942848,
            \"duration\": \"224.165442\",
            \"bit_rate\": \"32218\",
            \"nb_frames\": \"4827\",
            \"disposition\": {
                \"default\": 1,
                \"dub\": 0,
                \"original\": 0,
                \"comment\": 0,
                \"lyrics\": 0,
                \"karaoke\": 0,
                \"forced\": 0,
                \"hearing_impaired\": 0,
                \"visual_impaired\": 0,
                \"clean_effects\": 0,
                \"attached_pic\": 0\
            },\
            \"tags\": {
                \"creation_time\": \"2011-11-20 18:51:08\",
                \"language\": \"chi\",
                \"handler_name\": \"GPAC ISO Audio Handler\"
            }\
        }\
    ],\
    \"format\": {
        \"filename\": \"lie.mp4\",
        \"nb_streams\": 2,
        \"nb_programs\": 0,
        \"format_name\": \"mov,mp4,m4a,3gp,3g2,mj2\",
        \"format_long_name\": \"QuickTime / MOV\",
        \"start_time\": \"0.000000\",
        \"duration\": \"224.165000\",
        \"size\": \"20024577\",
        \"bit_rate\": \"714637\",
        \"probe_score\": 100,
        \"tags\": {
            \"major_brand\": \"isom\",
            \"minor_version\": \"1\",
            \"compatible_brands\": \"isomavc1\",
            \"creation_time\": \"2011-11-20 18:51:08\",
            \"album\": \"Yinyuetai\",
            \"artist\": \"yinyuetai.com\",
            \"comment\": \"Yinyuetai Inc\",
            \"date\": \"11/22/11 02:51:09\",
            \"title\": \" \"
        }\
    }\
}"

int main(int argc, char *argv[])
{
	cJSON *head = NULL, *streams = NULL, *video_item = NULL, *audio_item = NULL;
    head = cJSON_Parse(buf);
    if(head)
        streams = cJSON_GetObjectItem(head, "streams");
    else
        return -1; 
    int size = cJSON_GetArraySize(streams);
    if(size > 1)
    {   
        video_item = cJSON_GetArrayItem(streams, 0); 
        audio_item = cJSON_GetArrayItem(streams, 1); 
    }else
        return -2; 

    if(video_item)
        snprintf(v_code, v_size, "%s", cJSON_GetObjectItem(video_item, "codec_name")->valuestring);
    if(audio_item)
        snprintf(a_code, a_size, "%s", cJSON_GetObjectItem(audio_item, "codec_name")->valuestring);
        
    cJSON_Delete(head);

    printf("formats:\n*%s*\n", m_formats);

    return 0;
}

	return 0;
}
