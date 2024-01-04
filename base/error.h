// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef BASE_ERROR_H_
#define BASE_ERROR_H_

#include <stdexcept>
#include <string>

#include "base/base_export.h"
#include "base/base_types.h"

#define BASE(n) 5000 + n

enum Err {
  ERR_NONE = 0,        //正确
  ERR_FAILURE,         //失败
  ERR_UNKOWN,          //未知错误
  ERR_NOT_ENOUGH_MEM,  //内存不足

  ERR_OPEN_INNER,     //打开错误
  ERR_ARCHIVE_INNER,  //获取错误
  ERR_CREATE_INNER,   //创建错误
  ERR_SAVE_INNER,     //保存错误
  ERR_MATH_INNER,     //数学错误

  ERR_FUNC_INNER,  //函数内部错误

  ERR_INVALID_HANDLE,  //无效句柄
  ERR_INVALID_PARAM,   //无效参数
  ERR_INVALID_FILE,    //无效文件

  ERR_UNSUPPORTED,          //不支持
  ERR_UNSUPPORTED_DEVICE,   //不支持的驱动
  ERR_UNSUPPORTED_FMTS,     //不支持格式
  ERR_UNSUPPORTED_GEOTYPE,  //正确不支持几何类型

  ERR_DS_INNER,  //数据源内部错误
  ERR_DB_OPER,   // db操作错误
  ERR_SMF_OPER,  // smf操作错误

  ERR_ALREADYEXIST,  //警告对象已存在
};

typedef uint32_t RESULT;

/* define error code */
const RESULT kResultSuccess = 0;
const RESULT kResultFailed = 1;
const RESULT kResultInvalidHandle = BASE(2);
const RESULT kResultObjectNull = BASE(3);
const RESULT kResultIOError = BASE(4);
const RESULT kResultMemoryError = BASE(5);
const RESULT kResultTimeout = BASE(6);

namespace base {
class BASE_EXPORT Exception : public std::runtime_error {
 public:
  Exception(const std::string &e)
      : std::runtime_error(e), error_code_(kResultFailed) {}
  Exception(const char *s, uint32_t error_code = kResultFailed)
      : std::runtime_error(s), error_code_(error_code) {}
  Exception()
      : std::runtime_error((const char *)"Unknown error"),
        error_code_(kResultFailed) {}
  virtual ~Exception() throw() {}
  uint32_t error_code() const { return error_code_; }

 private:
  uint32_t error_code_;
};
}  // namespace base

#endif  // BASE_ERROR_H_
