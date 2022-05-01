#include"gui_base.h"
#include"chatroom_client.hpp"
#include"string_convert.hpp"


JNIEXPORT jlong JNICALL Java_gui_1base_create_1obj
(JNIEnv* env, jobject, jstring ip_address, jint port_num)
{
	const char* c_ip_address = env->GetStringUTFChars(ip_address, 0);

	return (jlong)new chatroom_client(AF_INET, SOCK_STREAM, IPPROTO_TCP, port_num, c_ip_address);
}

JNIEXPORT jint JNICALL Java_gui_1base_login
(JNIEnv* env, jobject, jlong ptr, jstring user_name, jstring passwd)
{
	const char* c_user_name = env->GetStringUTFChars(user_name, 0);
	const char* c_passwd = env->GetStringUTFChars(passwd, 0);

	return ((chatroom_client*)ptr)->login(c_user_name, c_passwd);
}

JNIEXPORT jboolean JNICALL Java_gui_1base_send_1msg
(JNIEnv* env, jobject, jlong ptr, jstring msg)
{
	string c_msg = jstring_to_string(env, msg);
	return ((chatroom_client*)ptr)->send_wrapper(c_msg.c_str());
}

JNIEXPORT jstring JNICALL Java_gui_1base_recv_1msg
(JNIEnv* env, jobject, jlong ptr)
{
	string msg = ((chatroom_client*)ptr)->recv_wrapper();
	return string_to_jstring(env, msg);
}

JNIEXPORT jstring JNICALL Java_gui_1base_recv_1history
(JNIEnv* env, jobject, jlong ptr)
{
	string history_msg = ((chatroom_client*)ptr)->recv_history();
	return string_to_jstring(env, history_msg);
}

JNIEXPORT jstring JNICALL Java_gui_1base_recv_1online_1user
(JNIEnv* env, jobject, jlong ptr)
{
	string user_msg = ((chatroom_client*)ptr)->recv_online_user();
	return string_to_jstring(env, user_msg);
}


//via command function
JNIEXPORT jboolean JNICALL Java_gui_1base_file_1send
(JNIEnv* env, jobject, jlong ptr, jstring msg) 
{
	const char* c_msg = env->GetStringUTFChars(msg, 0);

	return ((chatroom_client*)ptr)->call_command(c_msg);
}

//via command function
JNIEXPORT jboolean JNICALL Java_gui_1base_file_1path_1set
(JNIEnv* env, jobject, jlong ptr, jstring msg)
{
	const char* c_msg = env->GetStringUTFChars(msg, 0);

	return ((chatroom_client*)ptr)->call_command(c_msg);
}


//call send_wrapper
JNIEXPORT void JNICALL Java_gui_1base_send_1msg_1to
(JNIEnv* env, jobject, jlong ptr, jstring msg)
{
	string c_msg = jstring_to_string(env, msg);
	((chatroom_client*)ptr)->send_wrapper(c_msg.c_str());
}


//call send wrapper
JNIEXPORT void JNICALL Java_gui_1base_delete_1account
(JNIEnv* env, jobject, jlong ptr, jstring msg)
{
	const char* c_msg = env->GetStringUTFChars(msg, 0);
	((chatroom_client*)ptr)->send_wrapper(c_msg);
}


//call send wrapper
JNIEXPORT void JNICALL Java_gui_1base_reset_1passwd
(JNIEnv* env, jobject, jlong ptr, jstring msg)
{
	const char* c_msg = env->GetStringUTFChars(msg, 0);
	((chatroom_client*)ptr)->send_wrapper(c_msg);
}