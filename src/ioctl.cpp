#include <errno.h>
#include <sys/ioctl.h>
#include <napi.h>

using namespace Napi;

bool isInteger(const Napi::Value& val) {
  if (!val.IsNumber()) {
    return false;
  }

  double d = val.ToNumber().Unwrap().DoubleValue();
  return static_cast<double>(static_cast<int64_t>(d)) == d;
}

Napi::Number Ioctl(const Napi::CallbackInfo& info) {
  void* argp = NULL;
  Napi::Env env = info.Env();

  size_t length = info.Length();
  if ((length != 2) && (length != 3)) {
    Napi::Error::New(env, "ioctl only accepts 2 or 3 arguments").
        ThrowAsJavaScriptException();
    return Number();
  }

  if (!isInteger(info[0])) {
    Napi::Error::New(env, "Argument 0 Must be an Integer").
        ThrowAsJavaScriptException();
    return Number();
  }

  if (!isInteger(info[1])) {
    Napi::Error::New(env, "Argument 1 Must be an Integer").
        ThrowAsJavaScriptException();
    return Number();
  }

  if ((length == 3) && !info[2].IsUndefined()) {
    if (isInteger(info[2])) {
      argp = reinterpret_cast<void*>(info[2].ToNumber().Unwrap().Int32Value());
    } else if (info[2].IsBuffer()) {
      argp = info[2].As<Napi::Buffer<unsigned char>>().Data();
    } else {
      Napi::Error::New(env, "Argument 2 Must be an Integer or a Buffer").
        ThrowAsJavaScriptException();
      return Number();
    }
  }

  int fd = info[0].ToNumber().Unwrap().Int32Value();
  unsigned long request =
      static_cast<unsigned long>(info[1].ToNumber().Unwrap().DoubleValue());

  int res = ioctl(fd, request, argp);
  if (res < 0) {
    Napi::Error e = Napi::Error::New(env, "ioctl");
    e.Set("code", Napi::Number::New(env, errno));
    e.ThrowAsJavaScriptException();
    return Number();
  }

  return Napi::Number::New(env, res);
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "ioctl"),
                Napi::Function::New(env, Ioctl));
    return exports;
}

NODE_API_MODULE(ioctl, InitAll)

