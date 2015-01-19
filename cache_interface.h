/*
 * cache_namespace.h
 *
 *  Created on: 2012-8-13
 *      Author: jx.peng
 */

#ifndef CACHE_INTERFACE_H_
#define CACHE_INTERFACE_H_
#include <stdint.h>
#include <nshead.h>

/*
 * declare the cache operation type for dispatch different kinds of
 * cache items.
 */
typedef enum cache_type
{
    basicItem   = 0x0,
    finalItem   = 0x10,
    sFinalItem  = 0x11,
    clipFinal   = 0x12,
    sClipFinal  = 0x13,
    incrData    = 0x14,
    sIncrData   = 0x15,

    finalList   = 0x20,
    sFinalList  = 0x21,
    cmtFinalList = 0x22,

    mfFinal     = 0x30,
    sMfFinal    = 0x31,
    mediItem    = 0x32,
    sMediItem   = 0x33,

    finalXml    = 0x40,
    sFinalXml   = 0x41,

	finalIndividual = 0x50,
	finalSIndividual = 0x51
} cache_t;

typedef enum CACHE_ITEM_TYPE{
    FINAL_HTML = 0x1,
    FINAL_MAIN_HTML,
    FINAL_COOKIE,
    FINAL_LIST_PURE,
    FINAL_LIST_ALL
}cache_item_t;

/* operation id for basic cache operation*/
enum CacheCommand{
    CMD_CACHE_SEEK = 0,
    CMD_CACHE_ADD = 1,
    CMD_CACHE_DEL = 2,
    CMD_CACHE_RANGE_REMOVE_NAMESPACE = 3,
    CMD_CACHE_UPDATE_WITH_PREV_EXPIRE = 4,
    CMD_CACHE_APPEND = 5,                   //cache will handle it with lock
    CMD_CACHE_SEEK_LOCK = 6,                //cache will handle it with lock
    CMD_CACHE_ADD_LOCK = 7,                 //cache will handle it with lock
    CMD_CACHE_DEL_LOCK = 8,                 //cache will handle it with lock
    CMD_CACHE_INCREASE = 9,                 //regard it as integer(int64), cache will handle it with lock
    CMD_CACHE_TABLE_OP = 10,
    CMD_CACHE_OP = 11,                      //duplicate
    CMD_CACHE_MAX
};

enum CacheAppendCommand{
    CacheListAppend,
    CacheListAdd,
    CacheListDel,
    CacheListMix
};

enum CacheListMixType{
    listMixAdd,
    listMixDel
};

#define CMD_CACHE_LIST_APPEND ((CacheListAppend << 4) | CMD_CACHE_APPEND)
#define CMD_CACHE_LIST_ADD ((CacheListAdd << 4) | CMD_CACHE_APPEND)
#define CMD_CACHE_LIST_DEL ((CacheListDel << 4) | CMD_CACHE_APPEND)
#define CMD_CACHE_LIST_MIX ((CacheListMix << 4) | CMD_CACHE_APPEND)

enum CacheOpCommand{
    CacheStartDup,
    CacheStartTransfer,
    CacheDup,
    CacheTransfer
};

#define CMD_CACHE_OP_START_DUP ((CacheStartDup << 4) | CMD_CACHE_OP)
#define CMD_CACHE_OP_START_TRANSFER ((CacheStartTransfer << 4) | CMD_CACHE_OP)
#define CMD_CACHE_OP_DUP ((CacheDup << 4) | CMD_CACHE_OP)
#define CMD_CACHE_OP_TRANSFER ((CacheTransfer << 4) | CMD_CACHE_OP)


#define CMD_CACHE_MODIFY_BIT        1

/* cache operation id for full browsing */
#define CMD_CACHE_MODIFY_ADD            ((CMD_CACHE_MODIFY_BIT << 4) | CMD_CACHE_ADD)

#define CMD_CACHE_SEEK_FINAL            ((finalItem << 4) | CMD_CACHE_SEEK) /*0x101*/
#define CMD_CACHE_ADD_FINAL_ITEM	    ((finalItem << 4) | CMD_CACHE_ADD)  /*0102*/

#define CMD_CACHE_ADD_FINAL_LIST	    ((finalList << 4) | CMD_CACHE_ADD) /*0x202 */

#define CMD_CACHE_ADD_MF_FINAL          ((mfFinal << 4) | CMD_CACHE_ADD) /*0x302 */
/* for rewrite the intermediate results */
#define CMD_CACHE_ADD_MEDI              ((mediItem << 4) | CMD_CACHE_ADD) /*0x322*/
#define CMD_CACHE_SEEK_MF_FINAL         ((mfFinal << 4) | CMD_CACHE_SEEK) /*0x301 */

#define CMD_CACHE_SEEK_XML_FINAL        ((finalXml << 4) | CMD_CACHE_SEEK) /*0x401 */
#define CMD_CACHE_ADD_XML_FINAL         ((finalXml << 4) | CMD_CACHE_ADD) /*0x402 */
/* clip list */
#define CMD_CACHE_SEEK_CLIPS_FINAL      ((clipFinal << 4) | CMD_CACHE_SEEK) /*0x121*/

/* increase data */
#define CMD_CACHE_SEEK_INCR_DATA 		((incrData << 4) | CMD_CACHE_SEEK) // 0x141
#define CMD_CACHE_ADD_INCR_DATA	        ((incrData << 4) | CMD_CACHE_ADD)  /*0x142*/

/* final individuals */
#define CMD_CACHE_SEEK_FINAL_INDIVIDUAL ((finalIndividual << 4) | CMD_CACHE_SEEK) /* 0x501 */
#define CMD_CACHE_SEEK_FINAL_SINDIVIDUAL ((finalSIndividual << 4) | CMD_CACHE_SEEK) /* 0X511 */



/* cache operation id for simple browsing */
#define CMD_CACHE_SEEK_SFINAL		    ((sFinalItem << 4) | CMD_CACHE_SEEK) /*0x111  */
#define CMD_CACHE_ADD_FINAL_SITEM	    ((sFinalItem << 4) | CMD_CACHE_ADD) /*0x112 */

#define CMD_CACHE_ADD_FINAL_SLIST	    ((sFinalList << 4) | CMD_CACHE_ADD) /*0x212  */

#define CMD_CACHE_ADD_SMF_FINAL         ((sMfFinal << 4) | CMD_CACHE_ADD)  /*0x312  */
/* for rewrite the intermediate results for simple browsing */
#define CMD_CACHE_ADD_SMEDI             ((sMediItem << 4) | CMD_CACHE_ADD) /*0x332 */
#define CMD_CACHE_SEEK_SMF_FINAL        ((sMfFinal << 4) | CMD_CACHE_SEEK) /*0x311 */

#define CMD_CACHE_SEEK_SXML_FINAL       ((sFinalXml << 4) | CMD_CACHE_SEEK) /*0x411  */
#define CMD_CACHE_ADD_SXML_FINAL        ((sFinalXml << 4) | CMD_CACHE_ADD) /*0x412  */
/* clip list */
#define CMD_CACHE_SEEK_CLIPS_SFINAL     ((sClipFinal << 4) | CMD_CACHE_SEEK) //0x131

/* increase data */
#define CMD_CACHE_SEEK_SINCR_DATA 		((sIncrData << 4) | CMD_CACHE_SEEK) //0x151
#define CMD_CACHE_ADD_SINCR_DATA	    ((sIncrData << 4) | CMD_CACHE_ADD)  //0x152

/* commit list */
#define CMD_CACHE_SEEK_CMT_FINAL_LIST	((cmtFinalList << 4) | CMD_CACHE_SEEK)  //0x221


#define CMD_CACHE_END				0x900 /* end of the cmd */


const static char LIST_SPLITER[] = "\r\n";
const static uint32_t LIST_SPLITER_SIZE = sizeof(LIST_SPLITER) - 1;

const static char TABLE_ADD_KEY[] = "tableadd";
const static char TABLE_MAX_KEY[] = "tablemax";
const static char TABLE_KEYWORDS_KEY[] = "keyword";
const static char TABLE_TICK_KEY[] = "tick";
const static char TABLE_USER_SUB_KEY[] = "user_sub";
const static uint32_t TABLE_ADD_KEY_SIZE = sizeof(TABLE_ADD_KEY) - 1;
const static char TABLE_DEL_KEY[] = "tabledel";
const static uint32_t TABLE_DEL_KEY_SIZE = sizeof(TABLE_DEL_KEY) - 1;

enum CacheRetCode{
    CacheTableInputError = -7,
    CacheListResultEmtpy = -6, //in fact, it represent success
    CacheListMixParaError = -5,
    CacheListRmEmpty = -4,
    CacheAppendOverFlow = -3,
    CacheSizeError = -2,
    CacheOperationError = -1,
    CacheOperationSuccess = 0,//for compatible, must equal to CACHE_RET_HIT
    CacheSeekNoHit = 1 //for compatible, must equal to CACHE_RET_NOTHIT
};

/*
 * define the url tag for full and simple browsing
 */
/* full browsing */
#define cache_url_final_tag                         "{f"
#define cache_url_final_list_tag                    "{"
#define cache_url_mf_tag                            "{m"
#define cache_url_xml_final_tag                     "("

#define cache_url_final_list_char_tag               '{'
#define cache_url_xml_final_char_tag                '('

/* simple browsing */
#define cache_url_sfinal_tag                        "}f"
#define cache_url_final_slist_tag                   "}"
#define cache_url_smf_tag                           "}m"
#define cache_url_sxml_final_tag                    ")"

#define cache_url_final_slist_char_tag              '}'
#define cache_url_sxml_final_char_tag               ')'

/* common separator */
#define cache_url_sep                               "\n"

/** cookie url tag for cloud cookie storage */
#define cache_cookie_url_tag ".cookie_dxpack"

enum CacheNamespace{
	PlainNamespace = 1,
	RawImageNamespace = 2,
	ProcessedImageNamespace = 3,
	TopicNamespace = 4,
	SubscribeListNamespace = 5,
	WordFrequencyNamespace = 6,
	CrawlerNamespace = 7,
	SubscribeKwListNamespace = 8,
    FrameNameSpace = 9,//for frame
    FrameRangeNameSapce = 10,//for frame
    AssembleMetaNameSpace = 11,//for gate
    ReadingUpdaterNameSpace = 12,//for home user updater
    ReadingFinalRangeNameSpace = 13,//user will see it
    ReadingFinalBlockNameSpace = 14,//user will see it
    ReadingFinalGroupNameSpace = 15,//user will see it
    ReadingSecondRangeNameSpace = 16,
    ReadingSecondNameSpace = 17,
    ReadingNormalNameSpace = 18,
    ReadingNormalRangeNameSpace = 19,
    UserTableNameSpace = 25,
    KeyWordTableNameSpace = 26,
    SidTableNameSpace = 27,
    CacheNameSpaceMax = 40
};

#define _BDTC_MAX_URL_LEN 1024

struct cache_req_t{
    cache_req_t() : name_space(PlainNamespace){}
    //cache timeout, use default time if set to 0, otherwise ms
    uint32_t age;
    //for future, now not in use
    uint32_t bucket;
    //high 8bit treats as type:
    //1 : final result
    uint16_t cmd_id;
    uint16_t name_space;
    char key[_BDTC_MAX_URL_LEN];//url key
    char buf;//extra data if cmd is store
};

struct cache_res_t{
    short ret_no; //0 = hit, 1= no hit, 2=download <0 error
    bool b_body;//whether has extra data, nshead.len-sizeof(cache_res_t)
};

const static uint32_t CACHE_REQ_HEADER_SIZE = sizeof(cache_req_t);
const static uint32_t CACHE_RES_HEADER_SIZE = sizeof(cache_res_t);

const static uint32_t CACHE_REQ_CONTENT_MAX_SIZE = (8u<<20) - sizeof(cache_req_t) - sizeof(nshead_t);
const static uint32_t CACHE_RES_CONTENT_MAX_SIZE = (8u<<20) - sizeof(cache_res_t) - sizeof(nshead_t);

const static uint32_t NO_EXPIRE_TIME = 0xffffffff;

inline bool isCacheNamespaceLegal(int name_space){
    return name_space >= PlainNamespace && name_space < CacheNameSpaceMax;
}

inline bool isCacheCmdLegal(int cmd){
    return cmd >= CMD_CACHE_SEEK && cmd < CMD_CACHE_MAX;
}

inline bool canIgnoreKey(int cmd){
    cmd = cmd & 0x0F;
    if(cmd == CMD_CACHE_RANGE_REMOVE_NAMESPACE || cmd == CMD_CACHE_OP)
        return true;

    return false;
}

//below for compatible
#define CACHE_RET_HIT           0
#define CACHE_RET_NOTHIT        1
#define CACHE_RET_DOWNLOAD      2
#define CACHE_TIMEOUT           3
#define CACHE_RET_FINAL_COOKIE  4


#define CACHE_RETERR_INITIAL   -1
#define CACHE_RETERR_UNKNOW    -2

#endif /* CACHE_INTERFACE_H_ */
