/**
\file
\brief example the usage of memory surface 
\copyright Shenzhen Hisilicon Co., Ltd.
\date 2008-2018
\version draft
\author x57522
\date 2008-9-17
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hi_go.h"
#include "sample_displayInit.h"

/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/
HI_S32 main(HI_S32 argc, HI_CHAR* argv[])
{
    HI_S32 ret;
    HIGO_LAYER_INFO_S stLayerInfo;
    HI_HANDLE hLayer,hLayerSurface,hMemSurface;
    HIGO_BLTOPT_S stBlitOpt;
    HI_RECT stRect;
    HIGO_LAYER_E eLayerID = HIGO_LAYER_HD_0;

    /** initial */
    ret = Display_Init();
    if (HI_SUCCESS != ret)
    {
        return ret;
    }
    
    ret = HI_GO_Init();
    if (HI_SUCCESS != ret)
    {
        goto ERR1;
    }

    /** create graphic layer *///获取图层默认参数
    ret = HI_GO_GetLayerDefaultParam(eLayerID, &stLayerInfo);
    if (HI_SUCCESS != ret)
    {
        goto ERR2;
    }
    //根据默认参数，创建图层
    ret = HI_GO_CreateLayer(&stLayerInfo,&hLayer);
    if (HI_SUCCESS != ret)
    {
        goto ERR2;
    }

    /** get the graphic layer Surface *///获取图层的内存surface
    ret = HI_GO_GetLayerSurface (hLayer,&hLayerSurface); 
    if (HI_SUCCESS != ret)  
    {
       goto ERR3;
    }

    /** create memory Surface *///创建内存surface的长宽像素格式
    ret = HI_GO_CreateSurface(1024,768,HIGO_PF_1555,&hMemSurface);
    if (HI_SUCCESS != ret)
    {
       goto ERR3;
    }

    /** set the attribute of memory surface *///设置透明度
    ret = HI_GO_SetSurfaceAlpha(hMemSurface,128);
    if (HI_SUCCESS != ret)
    {
        goto ERR4; 
    }
    
    /** draw a rectangle on the memory surface *///矩形填充
    ret = HI_GO_FillRect(hMemSurface,NULL,0xFFFF0000,HIGO_COMPOPT_NONE);
    if (HI_SUCCESS != ret)
    {
       goto ERR4; 
    }
	getchar();
    /** move it to graphic layer Surface */
    stRect.x = stRect.y = 0;
    stRect.w = 1024;
    stRect.h = 768;
    memset(&stBlitOpt,0,sizeof(HIGO_BLTOPT_S));
    //stBlitOpt.EnablePixelAlpha = HI_TRUE;
    stBlitOpt.EnableGlobalAlpha = HI_TRUE;
    /** should set the pixel alpha mix mode when enable global alpha*/	
    stBlitOpt.PixelAlphaComp = HIGO_COMPOPT_SRCOVER;
	//位块搬移
    ret = HI_GO_Blit(hMemSurface,NULL,hLayerSurface,&stRect,&stBlitOpt);
    if (HI_SUCCESS != ret)
    {
       goto ERR4;   
    }

    /**fresh the graphic layer */ 
    ret = HI_GO_RefreshLayer(hLayer, NULL);
    if (HI_SUCCESS != ret)
    {
        goto ERR4; 
    }
    Printf("please input anykey to stop the test case\n");
    getchar();

ERR4:
    HI_GO_FreeSurface(hMemSurface);
ERR3:
    HI_GO_DestroyLayer(hLayer);
ERR2:
    HI_GO_Deinit();
ERR1:
    Display_DeInit();

    return ret;
}
