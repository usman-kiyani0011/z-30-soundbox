#include "lvgl/lvgl.h"
#include "lvgl/src/lv_lib_png/lv_png.h"
#include "libapi_xpos/inc/mfsdk_mem.h"
#include "libapi_xpos/inc/mfsdk_fs.h"
#include "libapi_xpos/inc/mfsdk_comm.h"
#include "libapi_xpos/inc/mfsdk_sys.h"
#include "tracedef.h"

#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BMPHeader;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BMPInfoHeader;
#pragma pack(pop)

#define APP_BMP_MAX_SIZE (330 *1024)
#define APP_RGB565_MAX_SIZE 320*480*2
#define APP_COMM_BUFFER 330*1024


static lv_img_dsc_t converted_img_var;
static unsigned char *s_app565 = NULL;
static unsigned char *s_AppCommBuffer = NULL;
static unsigned char *s_AppBmpBuffer = NULL;

/**
 * @brief
 * 
 * @return  Rgb 565 pointer
 */
void *AppGetRgb565(void)
{ 
	return s_app565; 
}
/**
 * @brief
 * 
 * @return 320* 480 *2 Rgb565 MAX length
 */
int AppGetRgb565Length(void)
{ 
	return APP_RGB565_MAX_SIZE; 
}

/**
 * @brief
 * 
 * @return pointer
 */
void* getBmpVar(void)
{
   return &converted_img_var;
}

/**
 * @brief
 * 
 * @param[in] uint32_t w
 * @param[in] uint32_t h
 * @param[in] const uint8_t * imgMap
 */
static void AppSetImageData(uint32_t w, uint32_t h, const uint8_t * imgMap)
{
  	converted_img_var.header.cf = LV_IMG_CF_TRUE_COLOR;
	converted_img_var.header.always_zero = 0;
	converted_img_var.header.reserved = 0;
	converted_img_var.header.w = w;
	converted_img_var.header.h = h;
	converted_img_var.data_size = w * h * LV_COLOR_SIZE / 8;
	converted_img_var.data = imgMap;
}



int AppBmp2Rgb565Ex(unsigned char *pBmp , unsigned char *outRgb565 ,int iRgb565)
{
	int iOffset = 0;

	// 读取BMP文件头
	BMPHeader bmpHeader;
	// 读取BMP信息头
    BMPInfoHeader bmpInfoHeader;
	
	memcpy((unsigned char *)&bmpHeader,pBmp,sizeof(BMPHeader));
	iOffset += sizeof(BMPHeader);

	memcpy((unsigned char *)&bmpInfoHeader,pBmp + iOffset,sizeof(BMPInfoHeader));
	iOffset += sizeof(BMPInfoHeader);
	
	int rowSize = ((bmpInfoHeader.biWidth * bmpInfoHeader.biBitCount + 31) / 32) * 4;
    // 为图像数据和调色板分配内存
    uint8_t *bmpData = NULL;//(uint8_t*)malloc(rowSize * height);
    uint8_t *palette = NULL;//(uint8_t*)malloc(4 * 256); // 每个颜色4字节	
	
	palette = pBmp + iOffset;
	iOffset += 1024;
	
	bmpData = pBmp + iOffset;
	
	int rgb56Length = 0;
	
// 转换并写入RGB565格式的数据
    for (int i = 0; i < bmpInfoHeader.biHeight; ++i) {
        for (int j = 0; j < bmpInfoHeader.biWidth; ++j) {
            // 获取像素的颜色索引
            uint8_t colorIndex = bmpData[i * rowSize + j];

            // 根据索引获取颜色值
            uint8_t red = palette[colorIndex * 4 + 2];
            uint8_t green = palette[colorIndex * 4 + 1];
            uint8_t blue = palette[colorIndex * 4];

            // 将RGB值转换为RGB565格式并写入输出文件
            uint16_t rgb565 = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
			memcpy(outRgb565+rgb56Length,(uint8_t*)&rgb565,sizeof(uint16_t));
			rgb56Length += sizeof(uint16_t);
        }
    }
	return rgb56Length;	
}


int App16Bmp2Rgb565(unsigned char *pBmp , unsigned char *outRgb565 ,int iRgb565)
{
	int iOffset = 0;

	// 读取BMP文件头
	BMPHeader bmpHeader;
	// 读取BMP信息头
    BMPInfoHeader bmpInfoHeader;
	
	memcpy((unsigned char *)&bmpHeader,pBmp,sizeof(BMPHeader));
	iOffset += sizeof(BMPHeader);

	memcpy((unsigned char *)&bmpInfoHeader,pBmp + iOffset,sizeof(BMPInfoHeader));
	iOffset += sizeof(BMPInfoHeader);

	unsigned char *pBmpImage = pBmp+bmpHeader.bfOffBits;
	int32_t Height = bmpInfoHeader.biHeight;	
    if (Height < 0) { Height = -Height; }
	int width = bmpInfoHeader.biWidth < 0 ? (-bmpInfoHeader.biWidth) : bmpInfoHeader.biWidth;
	
	int rowSize = (((bmpInfoHeader.biWidth*bmpInfoHeader.biBitCount / 8) + 3)/4) * 4;

	for(int rows = 0; rows < Height; rows++)
	{
		unsigned short *pd = (unsigned short*)(pBmpImage + rows*rowSize);
		for (int cols = 0; cols < width; cols++)
		{
			pd[cols] = 	((pd[cols] >> 8) & 0xFF) | ((pd[cols]<<8) & 0xFF00);		
		}
	}

	for (int y = 0; y < Height / 2; y++) {
        for (int x = 0; x < bmpInfoHeader.biWidth * 2; x += 2) {
            int topIndex = y * rowSize + x;
            int bottomIndex = (Height - y - 1) * rowSize + x;
            // 交换像素数据
            unsigned char temp[2];
            memcpy(temp, &pBmpImage[topIndex], 2);
            memcpy(&pBmpImage[topIndex], &pBmpImage[bottomIndex], 2);			
            memcpy(&pBmpImage[bottomIndex], temp, 2);			
        }
    }
	memcpy(outRgb565,pBmp+bmpHeader.bfOffBits,width*Height*2);
	return width*Height*2;
}

int AppBmp2Rgb565(unsigned char *pBmp , unsigned char *outRgb565 ,int iRgb565)
{
	int iOffset = 0;

	// 读取BMP文件头
	BMPHeader bmpHeader;
	// 读取BMP信息头
    BMPInfoHeader bmpInfoHeader;
	
	memcpy((unsigned char *)&bmpHeader,pBmp,sizeof(BMPHeader));
	iOffset += sizeof(BMPHeader);

	memcpy((unsigned char *)&bmpInfoHeader,pBmp + iOffset,sizeof(BMPInfoHeader));
	iOffset += sizeof(BMPInfoHeader);
	
	int32_t height = bmpInfoHeader.biHeight;
    if (height < 0) { height = -height; }
	
	 // 计算每行的字节数（以4字节对齐）
    int rowSize = ((bmpInfoHeader.biWidth * bmpInfoHeader.biBitCount + 31) / 32) * 4;

    // 为图像数据和调色板分配内存
    uint8_t *bmpData = NULL;//(uint8_t*)malloc(rowSize * height);
    uint8_t *palette = NULL;//(uint8_t*)malloc(4 * 256); // 每个颜色4字节	
	
	palette = pBmp + iOffset;
	iOffset += 1024;
	
	bmpData = pBmp + iOffset;	
	int rgb56Length = 0;	
	 // 转换并写入RGB565格式的数据（进行上下反转）
    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < bmpInfoHeader.biWidth; x++) {
            uint8_t index = bmpData[y * rowSize + x];
            uint8_t blue = palette[index * 4];
            uint8_t green = palette[index * 4 + 1];
            uint8_t red = palette[index * 4 + 2];

            // 将RGB值转换为RGB565格式并写入输出文件
            uint16_t rgb565 = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);	
			memcpy(outRgb565+rgb56Length,(uint8_t*)&rgb565,sizeof(uint16_t));
			rgb56Length += sizeof(uint16_t);
        }
    }
	return rgb56Length;
}

void AppUartRecv(void * param)
{
	unsigned char *buf = s_AppBmpBuffer;
	BMPHeader* bmpHeader = NULL;
	BMPInfoHeader *bmpInforHeader = NULL;
	u32 cTicks = 0;
	int iTotalLength = 0;

	while(1)
	{	
		int x = MfSdkCommUartRecv(MFSDK_COMM_UART_COM26,buf+iTotalLength,APP_COMM_BUFFER-iTotalLength,10);

		if(x > 0) 
		{ 	
			iTotalLength += x; 
			cTicks = MfSdkSysGetTick();
			continue;
		}
		if(iTotalLength >= sizeof(BMPHeader))
		{
			bmpHeader = (BMPHeader*)buf;
			if(bmpHeader->bfType != 0x4D42)
			{
				iTotalLength = 0;
				memset(buf,0,APP_COMM_BUFFER);
				continue;
			}
			if(bmpHeader->bfSize > APP_COMM_BUFFER)
			{
				iTotalLength = 0;
				memset(buf,0,APP_COMM_BUFFER);
				continue;
			}
			if(iTotalLength >=bmpHeader->bfSize)
			{
				iTotalLength = 0;
				// recv success
				bmpInforHeader = (BMPInfoHeader*)(buf + sizeof(BMPHeader));			
				int32_t height = bmpInforHeader->biHeight;
				int32_t width = bmpInforHeader->biWidth;
				int xx = 0;
				if(bmpInforHeader->biBitCount == 8)
				{
					if (height < 0) 
					{ 
						xx = AppBmp2Rgb565Ex(buf , AppGetRgb565(),APP_RGB565_MAX_SIZE);
					}
					else
					{
						xx = AppBmp2Rgb565(buf , AppGetRgb565(),APP_RGB565_MAX_SIZE);		
					}
				}
				else if(bmpInforHeader->biBitCount == 16)
				{
					xx = App16Bmp2Rgb565(buf, AppGetRgb565(),APP_RGB565_MAX_SIZE);
				}
				else
				{
					continue;
				}
				memset(buf,0,APP_COMM_BUFFER);				
				height = height < 0 ? (-height) : height;
				AppSetImageData(width,height,AppGetRgb565());
				lv_start_lock(1);
				AppDispBmp(getBmpVar());
				lv_start_lock(0);					
			}
		}

		if((MfSdkSysGetTick() - cTicks) > 3000)
		{
			//recv time out
			iTotalLength = 0;
			memset(buf,0,APP_COMM_BUFFER);
		}
		if(iTotalLength <= 0) 
		{ 
			memset(buf,0,APP_COMM_BUFFER);
			MfSdkSysSleep(100); 
		}
	}
}

void AppInitUartRecvTask(void)
{
	s_app565 = MfSdkMemMalloc(APP_RGB565_MAX_SIZE);
	APP_TRACE("s_app565:%p\r\n",s_app565);

	s_AppCommBuffer = MfSdkMemMalloc(APP_COMM_BUFFER);
	APP_TRACE("s_AppCommBuffer:%p\r\n",s_AppCommBuffer);
	
	s_AppBmpBuffer = MfSdkMemMalloc(APP_BMP_MAX_SIZE);
	APP_TRACE("s_AppBmpBuffer:%p\r\n",s_AppBmpBuffer);

	if(s_AppCommBuffer != NULL)
	{
		mf_usb_cdc_setfifo(s_AppCommBuffer,APP_COMM_BUFFER);
	}
	
	int x = MfSdkSysTaskCreate(AppUartRecv,3, NULL, 1024*4);

	APP_TRACE("MfSdkSysTaskCreate:%p\r\n",x);

}


