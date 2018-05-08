/*
 * =====================================================================================
 *       Filename:  swhttpplay.h
 *    Description:  
 *        Created:  2011年09月13日 09时28分43秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  wanghuan 
 *        Company:  sunniwell 
 * =====================================================================================
 */

#ifndef __SWHTTPPLAY_H__
#define __SWHTTPPLAY_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* httpserver start*/
int httpplay_init();

/* httpserver exit */
void httpplay_exit();

/* httpplay read page.xml */
int httpplay_readdir(char *path);

/* httpplay print media info */
int httpplay_media_printinfo();

/* httpplay image show effect */
int httpplay_imageshow_effect();

/* httpplay image show  */
bool httpplay_image_show(char *file);

/* httpplay list version  get */
int httpplay_listversion_get();

/* httpplay list version  set */
int httpplay_listversion_set( int version);
#ifdef __cplusplus
}
#endif

#endif /*end swhttpplay.h  */
