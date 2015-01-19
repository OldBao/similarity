#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <sys/types.h>
#include <stdint.h>
#include "cache_interface.h"


#define _CLIENT_UPLOAD_DATA_TRUNK_URL						1	//客户端上传数据--访问URL
#define _CLIENT_UPLOAD_DATA_TRUNK_HTTP_HEAD				2	//客户端上传数据--HTTP消息头
#define _CLIENT_UPLOAD_DATA_TRUNK_JS_FUNCTION				3	//客户端上传数据--JavaScript函数调用
#define _CLIENT_UPLOAD_DATA_TRUNK_FORM_HEAD				4	//客户端上传数据--HTTP消息FORM表单头
#define _CLIENT_UPLOAD_DATA_TRUNK_GET_BIG_IMG				5	//客户端上传数据--获取大图
#define _CLIENT_UPLOAD_DATA_TRUNK_GET_SECCODE_IMG			6	//客户端上传数据--获取验证码
#define _CLIENT_UPLOAD_DATA_TRUNK_INTERACTIVE				7	//客户端上传数据--交互请求
#define _CLIENT_UPLOAD_DATA_TRUNK_FORM_BASIC_SE			20	//客户端上传数据--Form简单字段
#define _CLIENT_UPLOAD_DATA_TRUNK_FORM_EDIT_SE_MULTI		21	//客户端上传数据--Form多行编辑字段
#define _CLIENT_UPLOAD_DATA_TRUNK_FORM_EDIT_SE_CONT		22	//客户端上传数据--Form编辑字段内容
#define _CLIENT_UPLOAD_DATA_TRUNK_FORM_UPLOAD_FILE_PROP	23	//客户端上传数据--Form文件上传属性
#define _CLIENT_UPLOAD_DATA_TRUNK_FORM_UPLOAD_DATA			24	//客户端上传数据--Form文件上传数据
#define _CLIENT_UPLOAD_DATA_TRUNK_CLIENT_DATA				25	//客户端上传数据--客户端数据（主要交互用)
#define _CLIENT_UPLOAD_DATA_TRUNK_INER_PARAM				26	//客户端上传数据--与服务器内部交互用的参数
#define _CLIENT_UPLOAD_DATA_TRUNK_IA_FILE_DATA				27	//客户端上传数据--文件（主要交互用)
#define _CLIENT_UPLOAD_DATA_TRUNK_CLIENT_DATA_ENCRYPTED	28	//客户端上传数据--客户端加密数据（主要交互用)
#define _CLIENT_UPLOAD_DATA_TRUNK_FORM_BASIC_SE2			30	//客户端上传数据--Form简单字段，代替20
#define _CLIENT_UPLOAD_DATA_TRUNK_USER_AGENT				31	//客户端上传的User-Agent信息
#define _CLIENT_UPLOAD_DATA_TRUNK_CLINT_FONT_INFO			32	//客户端上传的字体信息
#define _CLIENT_UPLOAD_DATA_TRUNK_HTTP_COOKIE				33  //客户端上传的Cookie信息
#define _CLIENT_UPLOAD_DATA_TRUNK_CLINT_SETTING   			34  //客户端上传的设置信息
#define _CLIENT_UPLOAD_DATA_TRUNK_PIC_BUNDLE   			35  //客户端批量图片请求信息
#define _CLIENT_UPLOAD_DATA_TRUNK_MSG_REPLY   				36  //客户回复弹出窗口回复结果信息
#define _CLIENT_UPLOAD_DATA_TRUNK_CLIENT_IP    			37  //客户端real ip信息
#define _CLIENT_UPLOAD_DATA_TRUNK_CLIENT_INFO    			38  //客户端宽度高度IMEI
#define _CLIENT_UPLOAD_DATA_TRUNK_SUBFRAME_INFO  			39  //ONLINE模式下，SUBFrameInfo
#define _CLIENT_UPLOAD_DATA_TRUNK_SUBSCRIBE_INFO  			40  //ONLINE模式下，SubScribeInfo

#define _CLIENT_UPLOAD_DATA_TRUNK_CHILD_FRAME_BEGIN		10025	//子frame开始标签
#define _CLIENT_UPLOAD_DATA_TRUNK_CHILD_FRAME_END			10026	//子frame结束标签


enum REQ_TYPE
{
    CUTE_LAYOUT,
    FULL_TXT_LAYOUT,
    FULL_PIC_LYOUT,
    MAINTOPIC_LAYOUT,
    INTEREST_SUBSCRIBE
};

#define CUTE_W_H_F_C_POSTFIX_FORMAT "%d^%d^%d^%d" //width height font collapse
#define CUTE_SPLIING_MARK "^"
#define CUTE_PLATFORM_W_H_F_C_POSTFIX_FORMAT "'%s^%d^%d^%d^%d^%d" //platform, width height font collapse online
#define CUTE_PAGE_FORMAT "|pageidx=%d^pagesize=%d"
#define CUTE_PAGE_FORMAT_NONIA "|pagesize=%d"
#define CUTE_REQ_PAGING_FLAG "|pageidx"
#define CBS_ONLINE_SUB_FRAME_FORMAT "/cbsonline=%d/fw=%d/fh=%d/originw=%d/originh=%d/rate=%.4f/atomic=%d/src=%s" // frame url, final w, h, fit font, collapse.............only for online mode
#define CBS_ONLINE_UUID_FORMAT "/uuid=%s"
#define CBS_ONLINE_TYPE "/cbsonline="
#define CBS_ONLINE_TAG "bd_cbs_online"
#define CBS_SUBSCRIBE_TAG "bd_cbs_subscribe"
#define CBS_ONLINE_BASE "%s/bd_cbs_online/ua=%s/font=%d"
#define CBS_UA_FORMAT "bd_%d_%d_%s_%s_%s"  //bd_320_240_pc78_1-0-12-4_s1
#define CUTE_PAGE_INDEX "pageidx"

#define CUTE_FIRST_PAGE_INDEX "1"
#define CUTE_FIRST_PAGE_INDEX_NUM 1

#define CUTE_INCREMENT_ADD_PATH "/div[1]"
#define CUTE_INCREMENT_RM_PATH "/div[1]"

#define CUTE_TURN_PAGE_FORM_IDX 50002
#define CUTE_TURN_PAGE_SELECT_IDX 50001
#define CUTE_TURN_PAGE_SUBMIT_IDX 50000
#define CUTE_TURN_PAGE_UUID_IDX 49999
//#define CUTE_TURN_PAGE_CURPAGE_IDX 49998
#define CUTE_TURN_PAGE_IDX_STEP 100

#define CUTE_DEFAULT_SIZE 12;

#define MAX_DNS_RES_NUM 8

#define CMD_TCCORE_PROCESS 100


#define _BDTC_IA_EVENT_NONE                 (0x0)
#define _BDTC_IA_EVENT_CLICK                (0x00000001)
#define _BDTC_IA_EVENT_SUBMIT               (0x00000002)
#define _BDTC_IA_EVENT_CHANGE               (0x00000004)
#define _BDTC_IA_EVENT_MOUSEOVER            (0x00000008)
#define _BDTC_IA_EVENT_MOUSEOUT             (0x00000010)
#define _BDTC_IA_EVENT_MOUSEMOVE            (0x00000020)
#define _BDTC_IA_EVENT_MOUSEDOWN            (0x00000040)
#define _BDTC_IA_EVENT_MOUSEUP              (0x00000080)
#define _BDTC_IA_EVENT_FOCUS                (0x00000100)
#define _BDTC_IA_EVENT_BLUR                 (0x00000200)

#define _BDTC_TC_RECT "tc_rect"
#define _BDTC_TC_RECT_FORMAT "%d,%d,%d,%d"
#define _BDTC_TC_ORIGIN_RECT "tc_originrect"
#define _BDTC_TC_ORIGIN_RECT_FORMAT "%d,%d"
#define _BDTC_TC_XPADDING_RECT "tc_xpadding"
#define _BDTC_TC_YPADDING_RECT "tc_ypadding"
#define _BDTC_TC_PADDING_RECT_FORMAT "%d,%d"
#define _BDTC_TC_RATE "tc_rate"
#define _BDTC_CUTE_BGIMG_URL_FORMAT "%s?tc-res=8&tc_clip=%s&url=%s"
#define _BDTC_CUTE_BGIMG_URL_FORMAT_STRETCH "%s?tc-res=8&tc_clip=%s&tc_rect=%d_%d&url=%s"
#define _BDTC_CUTE_BGIMG_URL_FORMAT_WITH_RATE "%s?tc-res=8&tc_clip=%s&tc_rate=%.4f&url=%s"
#define _BDTC_CUTE_BGIMG_URL_FORMAT_STRETCH_WITH_RATE "%s?tc-res=8&tc_clip=%s&tc_rect=%d_%d&tc_rate=%.4f&url=%s"
#define _BDTC_CUTE_IMG_URL_FORMAT "%s?tc-res=8&url=%s"
#define _BDTC_CUTE_IMG_URL_FORMAT_WITH_RATE "%s?tc-res=8&tc_rate=%.4f&url=%s"
#define _BDTC_CUTE_STRETCH_STR "tc_stretch"
#define _BDTC_CUTE_STRETCH_NORMAL 0
#define _BDTC_CUTE_STRETCH_NINE 1
#define _BDTC_TC_ONLINE_STYLE "display:block;position:absolute;left:%dpx;top:%dpx;width:%dpx;height:%dpx;"

#define CBS_HTTP_HEADER_TIME_FORMAT "%s, %02d %s %d %02d:%02d:%02d GMT"  //Mon, 04 Jun 2012 09:13:16 GMT

#define CBS_HTML_CACHE_IDENTIFY 4785

enum BdFontLevel{
	BdFontSmall = 1,
	BdFontMid,
	BdFontBig
};

#define CRAWLER_CACHE_CONTROL "Crawler-cache-control"
enum CRAWLER_CACHE_CONTROL_TYPE{
	CRAWLER_CACHE_DEFAULT = 0,
	CRAWLER_CACHE_ONLY_SEEK,
	CRAWLER_CACHE_ONLY_ADD,
	CRAWLER_CACHE_NONE
};

struct crawler_req_t{
	short cmd_id; //1 = crawler main page.
	char req[_BDTC_MAX_URL_LEN];// request header
	char buf;// body buffer if exists
};

struct crawler_res_t{
	short ret_no; //0 = success
	int  size;//return buffer
};

struct tc_core_req_t{
#ifdef __TC_CORE_REQ_EXT__
	bool b_add_cache;//whether to cache, default true
	bool b_from_cache;//whether the data from cache, default true
	bool b_notifycache;//whether html page should be return, default false
	char	 cacheIP[3][32];//CacheIP list, no more than 3
	u_int32_t cachePort;//cache port
#endif
    char req_id;
	char cache_key[512];//cache key
	u_int32_t buf_len;// length of URL,key->value in datapack
#ifdef __TC_CORE_REQ_EXT__
	char buf;//datapack buffer
#endif
};

struct core_res_t{
	u_int32_t ret_no;//0=succ, otherwise fail
	u_int32_t age;//cache time, 0 for not to cache, otherwise cache time(ms)
	char content_type[64];//content-Type value
	char http_header_ext[512];//extra data in http response header
};

struct dns_res_t{
	unsigned int sign[2];
	int     h_addrtype;     /* host address type */
	int     h_length;       /* length of address */
	int 	res_num;//num of h_addr_list
	char    h_addr_list[MAX_DNS_RES_NUM][32];//use to store result, no more then 32
};


enum CBSOnlineType{
	CBSOnlineCuteNormal = 1,
	CBSOnlineCuteSubFrame = 2,
	CBSOnlineCuteInteractive = 3,
	CBSOnlineFullNormal = 4,
	CBSOnlineFullSubFrame = 5,
	CBSOnlineFullInteractive = 6,
	CBSOnlineMainTopic = 7,
	CBSOnlineSubscribe = 8,
	CBSOnlineTypeNum
};

enum CoreReqType
{
	CoreReqErrorType = 0,
	CoreReqNormal = 1,
	CoreReqSubScribe = 2
};

#endif



