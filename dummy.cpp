#define WIN32
#define _AMD64_

#include <windows.h>
#include <shobjidl_core.h>
#undef max
#undef min
#undef clamp
#include <windef.h>
#include "alice/conversion.hpp"

#define LEAN_EXPORTING
#include <lean/lean.h>

#include <cstdio>
#include <cstdlib>
#define GLM_FORCE_SWIZZLE
// #define GLEW_STATIC

#include <assert.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "SOIL2.h"

std::wstring open_path_selection_dialog() {
	IFileOpenDialog* DIALOG;
	auto DIALOG_RESULT = CoCreateInstance(
		CLSID_FileOpenDialog,
		NULL,
		CLSCTX_ALL,
		IID_IFileOpenDialog,
		reinterpret_cast<void**>(&DIALOG)
	);
	if(FAILED(DIALOG_RESULT)) {
		return L"";
	}

	// if (base_game) {
	// DIALOG->SetClientGuid(winapi::UUID_open_base_game);
	// } else {
	// DIALOG->SetClientGuid(winapi::UUID_open_mod);
	// }

	DIALOG->SetOptions(FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST | FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM | FOS_PICKFOLDERS);

	DIALOG_RESULT = DIALOG->Show(NULL);
	if(FAILED(DIALOG_RESULT)) {
		DIALOG->Release();
		return L"";
	}

	IShellItem* ITEM;
	DIALOG_RESULT = DIALOG->GetResult(&ITEM);
	if(FAILED(DIALOG_RESULT)) {
		DIALOG->Release();
		return L"";
	}

	//  STORE AND CONVERT THE FILE NAME
	PWSTR RETRIEVED_PATH;
	DIALOG_RESULT = ITEM->GetDisplayName(
		SIGDN_FILESYSPATH,
		&RETRIEVED_PATH
	);
	if(FAILED(DIALOG_RESULT)) {
		ITEM->Release();
		DIALOG->Release();
		return L"";
	}

	std::wstring path(RETRIEVED_PATH);
	CoTaskMemFree(RETRIEVED_PATH);
	ITEM->Release();
	DIALOG->Release();
	return path;
}

static void error_callback(int error, const char* description)
{
	fprintf(stdout, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
	ImGuiIO& io = ImGui::GetIO();
	io.AddMouseButtonEvent(button, action == GLFW_PRESS);
}

GLFWwindow* init_glfw_window() {

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(-1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	GLFWwindow* window;
	float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
	window = glfwCreateWindow(1280 * main_scale, 960 * main_scale, "DearImGuiLean", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(-1);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glewExperimental = GL_TRUE;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui::StyleColorsLight();

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(main_scale);
	style.FontScaleDpi = main_scale;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	printf("HELLO IMGUI\n");

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		if (err != GLEW_ERROR_NO_GL_VERSION)
			exit(-1);
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
	if (!GLEW_VERSION_3_3) {
		exit(-1);
	}

	fprintf(stdout, "Glew is ready\n");
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	fprintf(stdout, "Callback is ready\n");

	glClearColor(1.f, 1.f, 1.f, 1.f);

	printf("%p\n", window);

	return window;
}


void shutdown_glfw_window(GLFWwindow* window) {

	fprintf(stdout, "Shutdown Imgui\n");
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	fprintf(stdout, "Shutdown glfw\n");
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(0);
}


static void generic_finalize(void * obj) {
	fprintf(stdout, "Fin\n");
}

static void generic_foreach(void *, b_lean_obj_arg) {
	fprintf(stdout, "For each\n");
}


static lean_external_class *glfw_window_class = NULL;
static lean_external_class *image_class = NULL;


extern "C" LEAN_EXPORT lean_object * init_lean_classes(lean_object *) {
	fprintf(stdout, "Create classes\n");

	glfw_window_class = lean_register_external_class(
		&generic_finalize,
		&generic_foreach
	);
	fprintf(stdout, "GLFWwindow class has been created\n");

	image_class = lean_register_external_class(
		&generic_finalize,
		&generic_foreach
	);
	fprintf(stdout, "Image class has been created\n");

	return lean_io_result_mk_ok(lean_box(0));
}

static inline lean_obj_res GLFWwindow_to_lean(GLFWwindow * s) {
	fprintf(stdout, "Try to build lean object of window class\n");
	printf("%p\n", glfw_window_class);
	auto res =  lean_io_result_mk_ok(lean_alloc_external(glfw_window_class, s));
	printf("Done!\n");
	return res;
}
static inline GLFWwindow* lean_to_GLFWwindow(b_lean_obj_arg s) {
	return static_cast<GLFWwindow *>(lean_get_external_data(s));
}

extern "C" LEAN_EXPORT lean_obj_res lean_init_glfw_window() {
	return GLFWwindow_to_lean(init_glfw_window());
}
extern "C" LEAN_EXPORT lean_obj_res lean_shutdown_glfw_window(lean_obj_arg w) {
	shutdown_glfw_window(lean_to_GLFWwindow(w));
	return lean_io_result_mk_ok(lean_box(0));
}
extern "C" LEAN_EXPORT lean_obj_res lean_start_frame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	return lean_io_result_mk_ok(lean_box(0));
}
extern "C" LEAN_EXPORT lean_obj_res lean_end_frame() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	return lean_io_result_mk_ok(lean_box(0));
}
extern "C" LEAN_EXPORT lean_obj_res lean_swap_buffers(lean_obj_arg w) {
	glfwSwapBuffers(lean_to_GLFWwindow(w));
	return lean_io_result_mk_ok(lean_box(0));
}
extern "C" LEAN_EXPORT lean_obj_res lean_should_close(lean_obj_arg lean_w) {
	auto w  = (GLFWwindow*) (lean_get_external_data(lean_w));
	auto result = glfwWindowShouldClose(w);
	return lean_io_result_mk_ok(lean_box(result));
}

extern "C" LEAN_EXPORT lean_obj_res lean_poll_events() {
	glfwPollEvents();
	return lean_io_result_mk_ok(lean_box(0));
}

extern "C" LEAN_EXPORT lean_obj_res imgui_begin(b_lean_obj_arg label) {
	auto c_label = lean_string_cstr(label);
	ImGui::Begin(c_label);
	return lean_io_result_mk_ok(lean_box(0));
}

extern "C" LEAN_EXPORT lean_obj_res imgui_text(b_lean_obj_arg text) {
	auto c_text = lean_string_cstr(text);
	ImGui::Text("%s", c_text);

	return lean_io_result_mk_ok(lean_box(0));
}

extern "C" LEAN_EXPORT lean_obj_res imgui_button(b_lean_obj_arg text) {
	auto c_text = lean_string_cstr(text);
	auto pressed = ImGui::Button(c_text);
	return lean_io_result_mk_ok(lean_box(pressed));
}

extern "C" LEAN_EXPORT lean_obj_res imgui_end() {
	ImGui::End();
	return lean_io_result_mk_ok(lean_box(0));
}

extern "C" LEAN_EXPORT lean_obj_res draw_thing() {
	ImGui::Begin("Hello lean4ik");
	ImGui::Text("Test");
	ImGui::End();
	return lean_io_result_mk_ok(lean_box(0));
}


extern "C" LEAN_EXPORT lean_obj_res lean_glClear(uint32_t mask) {
	glClear(mask);
	return lean_io_result_mk_ok(lean_box(0));
}

extern "C" LEAN_EXPORT lean_obj_res lean_get_path() {
	auto path = open_path_selection_dialog();
	auto path_utf8 = conversions::w_to_u8(path);
	auto lean_str = lean_mk_string(path_utf8.c_str());
	return lean_io_result_mk_ok(lean_str);
}

struct image {
	int32_t w;
	int32_t h;
	int32_t channels;
	uint8_t* data;
};
extern "C" LEAN_EXPORT lean_obj_res lean_read_image(b_lean_obj_arg path) {
	printf("Allocate image array");
	auto c_text = lean_string_cstr(path);
	auto img = new image;
	img->data = SOIL_load_image(
		c_text,
		&img->w,
		&img->h,
		&img->channels,
		SOIL_LOAD_AUTO
	);
	auto img_boxed =  lean_io_result_mk_ok(
		lean_alloc_external(image_class, img)
	);

	return img_boxed;
}
extern "C" LEAN_EXPORT int32_t lean_image_width(lean_obj_arg img) {
	auto data  = (image*) (
		lean_get_external_data(img)
	);
	return data->w;
}
extern "C" LEAN_EXPORT int32_t lean_image_height(lean_obj_arg img) {
	auto data  = (image*) (
		lean_get_external_data(img)
	);
	return data->h;
}
extern "C" LEAN_EXPORT int32_t lean_image_channels(lean_obj_arg img) {
	auto data  = (image*) (
		lean_get_external_data(img)
	);
	return data->channels;
}


extern "C" LEAN_EXPORT lean_obj_res lean_glGenTextures() {
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	return lean_io_result_mk_ok(lean_box(texture_id));
}


extern "C" LEAN_EXPORT lean_obj_res lean_glBindTexture(int target, int texture_id) {
	glBindTexture(target, texture_id);
	return lean_io_result_mk_ok(lean_box(0));
}
extern "C" LEAN_EXPORT lean_obj_res lean_glTexParameteri(int target, int property, int param) {
	glTexParameteri(target, property, param);
	return lean_io_result_mk_ok(lean_box(0));
}
extern "C" LEAN_EXPORT lean_obj_res lean_load_image_to_gpu(lean_obj_arg img_box) {
	auto data  = (image*) (
		lean_get_external_data(img_box)
	);
	auto format = GL_RGBA;
	if (data->channels == 3) {
		format = GL_RGB;
	}
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		format,
		data->w,
		data->h,
		0,
		format,
		GL_UNSIGNED_BYTE,
		data->data
	);
	return lean_io_result_mk_ok(lean_box(0));
}

extern "C" LEAN_EXPORT lean_obj_res lean_display_image(int32_t texture_id_box, int32_t width, int32_t height) {
	ImGui::Image(texture_id_box, {(float)width, (float)height});
	return lean_io_result_mk_ok(lean_box(0));
}