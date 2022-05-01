#pragma once

#include<string>
#include"jni.h"

using std::string;

const char* CODE = "GBK";

jstring string_to_jstring(JNIEnv* env, const string& str)
{
    jclass jstrObj = env->FindClass("java/lang/String");
    jmethodID methodId = env->GetMethodID(jstrObj, "<init>", "([BLjava/lang/String;)V");
    jbyteArray byteArray = env->NewByteArray(str.length());
    jstring encode = env->NewStringUTF(CODE);
    env->SetByteArrayRegion(byteArray, 0, str.length(), (jbyte*)str.c_str());

    return (jstring)env->NewObject(jstrObj, methodId, byteArray, encode);
}


string jstring_to_string(JNIEnv* env, jstring jstr)
{
    char* c_str = nullptr;
    string res;

    jclass jstrObj = env->FindClass("java/lang/String");
    jmethodID methodId = env->GetMethodID(jstrObj, "getBytes", "(Ljava/lang/String;)[B");
    jstring encode = env->NewStringUTF(CODE);
    jbyteArray byteArray = (jbyteArray)env->CallObjectMethod(jstr, methodId, encode);
    jsize arrayLen = env->GetArrayLength(byteArray);
    jbyte* arrayByte = env->GetByteArrayElements(byteArray, JNI_FALSE);

    if (arrayLen > 0)
    {
        c_str = (char*)malloc((size_t)arrayLen + 1);

        if (c_str != nullptr)
        {
            memcpy(c_str, arrayByte, arrayLen);
            c_str[arrayLen] = 0;
        }
    }
    env->ReleaseByteArrayElements(byteArray, arrayByte, 0);

    if (c_str != nullptr)
    {
        res.assign(c_str);
        free(c_str);
    }

    return res;
}