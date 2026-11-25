#include <algorithm>
#include <optional>
#include <vector>
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

std::optional<std::wstring> open_save_file_dialog() {
	IFileSaveDialog* DIALOG;
	auto DIALOG_RESULT = CoCreateInstance(
		CLSID_FileSaveDialog,
		NULL,
		CLSCTX_ALL,
		IID_IFileSaveDialog,
		reinterpret_cast<void**>(&DIALOG)
	);
	if(FAILED(DIALOG_RESULT)) {
		return std::nullopt;
	}

	_COMDLG_FILTERSPEC filter;
	filter.pszName = L"All";
	filter.pszSpec = L"*.*";

	_COMDLG_FILTERSPEC FILE_TYPES[1] {filter};

	DIALOG_RESULT = DIALOG->Show(NULL);
	if(FAILED(DIALOG_RESULT)) {
		DIALOG->Release();
		return std::nullopt;
	}

	IShellItem* ITEM;
	DIALOG_RESULT = DIALOG->GetResult(&ITEM);
	if(FAILED(DIALOG_RESULT)) {
		DIALOG->Release();
		return std::nullopt;
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
		return std::nullopt;
	}

	std::wstring path(RETRIEVED_PATH);
	CoTaskMemFree(RETRIEVED_PATH);
	ITEM->Release();
	DIALOG->Release();
	return path;
}

std::wstring open_image_selection_dialog() {
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

	DIALOG->SetDefaultExtension(L"tga");

	_COMDLG_FILTERSPEC FILTER_JPEG;
	FILTER_JPEG.pszName = L"JPG (*.jpg,*.jpeg)";
	FILTER_JPEG.pszSpec = L"*.jpg;*.jpeg";

	_COMDLG_FILTERSPEC FILTER_PNG;
	FILTER_PNG.pszName = L"PNG (*.png)";
	FILTER_PNG.pszSpec = L"*.png";

	_COMDLG_FILTERSPEC FILTER_TGA;
	FILTER_TGA.pszName = L"TGA (*.tga)";
	FILTER_TGA.pszSpec = L"*.tga";

	_COMDLG_FILTERSPEC FILE_TYPES[3] {FILTER_PNG, FILTER_JPEG, FILTER_TGA};

	DIALOG->SetFileTypes(3, FILE_TYPES);
	DIALOG->SetOptions(FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST | FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM);

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
	DIALOG_RESULT = ITEM->GetDisplayName(SIGDN_FILESYSPATH, &RETRIEVED_PATH);
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


struct image {
	int32_t w;
	int32_t h;
	int32_t channels;
	uint8_t* data;
};

static void image_finalize(void * obj) {
	auto data  = (image*) (obj);

	delete[] data->data;
	delete data;

	fprintf(stdout, "Image deallocated\n");
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
extern "C" LEAN_EXPORT lean_obj_res lean_get_image_path() {
	auto path = open_image_selection_dialog();
	auto path_utf8 = conversions::w_to_u8(path);
	auto lean_str = lean_mk_string(path_utf8.c_str());
	return lean_io_result_mk_ok(lean_str);
}
extern "C" LEAN_EXPORT lean_obj_res lean_get_save_path() {
	auto path = open_save_file_dialog();
	if (path) {
		auto path_utf8 = conversions::w_to_u8(path.value());
		auto lean_str = lean_mk_string(path_utf8.c_str());
		return lean_io_result_mk_ok(lean_str);
	}
	lean_obj_arg error_message = lean_mk_string("Invalid saved file selection.");
	lean_obj_arg io_error = lean_mk_io_user_error(error_message);
	return lean_io_result_mk_error(io_error);
}
extern "C" LEAN_EXPORT lean_obj_res lean_write_dds(b_lean_obj_arg path, b_lean_obj_arg img) {
	auto c_text = lean_string_cstr(path);
	printf("Write DDS image to %s\n", c_text);
	auto data  = (image*) (
		lean_get_external_data(img)
	);
	SOIL_save_image(
		c_text,
		SOIL_SAVE_TYPE_DDS,
		data->w,
		data->h,
		data->channels,
		data->data
	);

	return lean_io_result_mk_ok(lean_box(0));
}
extern "C" LEAN_EXPORT lean_obj_res lean_read_image(b_lean_obj_arg path) {
	printf("Allocate image array\n");
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

struct Lab {float L; float a; float b;};
struct LabAlpha {Lab lab; float alpha;};
struct RGB {float r; float g; float b;};

float dist2(Lab a, Lab b) {
	return (a.L - b.L) * (a.L - b.L) + (a.a - b.a) * (a.a - b.a) + (a.b - b.b) * (a.b - b.b);
}

struct contrast_pair {
	int i;
	int j;
};

// https://bottosson.github.io/posts/oklab/
Lab linear_srgb_to_oklab(RGB c)
{
	float l = 0.4122214708f * c.r + 0.5363325363f * c.g + 0.0514459929f * c.b;
	float m = 0.2119034982f * c.r + 0.6806995451f * c.g + 0.1073969566f * c.b;
	float s = 0.0883024619f * c.r + 0.2817188376f * c.g + 0.6299787005f * c.b;

	float l_ = cbrtf(l);
	float m_ = cbrtf(m);
	float s_ = cbrtf(s);

	return {
		0.2104542553f*l_ + 0.7936177850f*m_ - 0.0040720468f*s_,
		1.9779984951f*l_ - 2.4285922050f*m_ + 0.4505937099f*s_,
		0.0259040371f*l_ + 0.7827717662f*m_ - 0.8086757660f*s_,
	};
}

RGB oklab_to_linear_srgb(Lab c)
{
	float l_ = c.L + 0.3963377774f * c.a + 0.2158037573f * c.b;
	float m_ = c.L - 0.1055613458f * c.a - 0.0638541728f * c.b;
	float s_ = c.L - 0.0894841775f * c.a - 1.2914855480f * c.b;

	float l = l_*l_*l_;
	float m = m_*m_*m_;
	float s = s_*s_*s_;

	return {
		+4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
		-1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
		-0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s,
	};
}

extern "C" LEAN_EXPORT lean_obj_res downscale_image(
	lean_obj_arg source_box,
	int size_x, int size_y
) {
	printf("Downscaling...\n");
	auto source  = (image*) (
		lean_get_external_data(source_box)
	);

	assert (source->channels == 4);

	std::vector<Lab> lab;
	std::vector<Lab> lab_smoothed;
	std::vector<float> alpha;
	lab.resize(source->h * source->w);
	alpha.resize(source->h * source->w);
	float max_lightness = 0.f;
	for (int i = 0; i  < (source->h) * (source->w); i++) {
		float r = source->data[i * source->channels + 0] / 255.f;
		float g = source->data[i * source->channels + 1] / 255.f;
		float b = source->data[i * source->channels + 2] / 255.f;
		float a = source->data[i * source->channels + 3] / 255.f;
		lab[i] = linear_srgb_to_oklab({r, g, b});
		alpha[i] = a;
		max_lightness = std::max(max_lightness, lab[i].L);
	}

	std::vector<contrast_pair> contrasts {};
	for (int i = 0; i  < (source->h) * (source->w); i++) {
		if (i + 1 < source->w) {
			contrasts.push_back({i, i + 1});
		}
		if (i + source->w < (source->h) * (source->w)) {
			contrasts.push_back({i, i + source->w});
		}
	}

	std::sort(contrasts.begin(), contrasts.end(), [&](contrast_pair& a, contrast_pair& b) {
		auto a_value = dist2(lab[a.i], lab[a.j]);
		auto b_value = dist2(lab[b.i], lab[b.j]);
		return std::abs(a_value) < std::abs(b_value);
	});

	auto result_image_data = new uint8_t[size_x * size_y * 4];
	auto result = new image;
	result->data = result_image_data;
	result->w = size_x;
	result->h = size_y;
	result->channels = 4;

	std::vector<Lab> result_lab {};
	std::vector<Lab> result_lab_smooth {};
	std::vector<float> result_a {};
	result_lab.resize(size_x * size_y);
	result_lab_smooth.resize(size_x * size_y);
	result_a.resize(size_x * size_y);
	float scale_x = float(size_x) / float(source->w);
	float scale_y = float(size_y) / float(source->h);

	int kernel_x = floorf(1.f / scale_x);
	int kernel_y = floorf(1.f / scale_y);

	auto sample_smooth = [&](float x, float y) {
		Lab total {};
		float local_alpha = 0.f;
		float total_weight = 0.f;
		for (int k_x = -kernel_x + 1; k_x < kernel_x; k_x++) {
			for (int k_y = -kernel_y + 1; k_y < kernel_y; k_y++) {
				auto sx = int(x / scale_x + k_x);
				auto sy = int(y / scale_y + k_y);

				if (sx < 0) continue;
				if (sy < 0) continue;
				if (sx >= source->w) continue;
				if (sy >= source->h) continue;

				auto pixel = sx + sy * source->w;
				auto dist2 = ((float)sx - x / scale_x) * ((float)sx - x / scale_x) + ((float)sy - y / scale_y) * ((float)sy - y / scale_y);
				auto local_weight = expf(-dist2);

				total.L += lab[pixel].L * local_weight;
				total.a += lab[pixel].a * local_weight;
				total.b += lab[pixel].b * local_weight;
				local_alpha += alpha[pixel] * local_weight;
				total_weight += local_weight;
			}
		}
		LabAlpha result {
			{
				total.L / total_weight,
				total.a / total_weight,
				total.b / total_weight
			},
			local_alpha / total_weight
		};

		return result;
	};

	for (auto p : contrasts) {
		if (p.j == p.i + 1) {
			int y = p.i / source->w;
			int x = p.i - y * source->w;
			auto target_x = int((float)x * scale_x);
			auto target_y = int((float)y * scale_y);
			auto target_pixel = target_x + target_y * size_x;

			if (target_x + 1 < size_x) {
				result_lab[target_pixel] = lab[p.i];
				result_lab[target_pixel + 1] = lab[p.j];

				result_a[target_pixel] = alpha[p.i];
				result_a[target_pixel + 1] = alpha[p.j];
			}
		}

		if (p.j == p.i + source->w) {
			int y = p.i / source->w;
			int x = p.i - y * source->w;
			auto target_x = int((float)x * scale_x);
			auto target_y = int((float)y * scale_y);
			auto target_pixel = target_x + target_y * size_x;

			if (target_y + 1 < size_y) {
				result_lab[target_pixel] = lab[p.i];
				result_lab[target_pixel + size_x] = lab[p.j];

				result_a[target_pixel] = alpha[p.i];
				result_a[target_pixel + size_x] = alpha[p.j];
			}
		}
	}

	for (int pixel = 0; pixel < size_x * size_y; pixel++) {
		auto ll = result_lab[pixel];

		int y = pixel / size_x;
		int x = pixel - y * size_x;

		auto lls = sample_smooth(x, y);

		Lab ll_average = {
			(ll.L + lls.lab.L) / 2.f,
			(ll.a + lls.lab.a) / 2.f,
			(ll.b + lls.lab.b) / 2.f,
		};

		auto rgb = oklab_to_linear_srgb(ll_average);
		auto a = lls.alpha;

		result_image_data[pixel * 4 + 0] = uint8_t(rgb.r * 255.f);
		result_image_data[pixel * 4 + 1] = uint8_t(rgb.g * 255.f);
		result_image_data[pixel * 4 + 2] = uint8_t(rgb.b * 255.f);
		result_image_data[pixel * 4 + 3] = uint8_t(a * 255.f);
	}

	assert(_CrtCheckMemory());
	printf("Apply aggregated patches\n");

	auto img_boxed =  lean_io_result_mk_ok(
		lean_alloc_external(
			image_class,
			result
		)
	);

	printf("Downscaling done\n");
	return img_boxed;
}

extern "C" LEAN_EXPORT lean_obj_res paste_image(
	lean_obj_arg source_box,
	lean_obj_arg destination_box,
	int destination_x, int destination_y
) {
	auto source  = (image*) (
		lean_get_external_data(source_box)
	);
	auto destination  = (image*) (
		lean_get_external_data(destination_box)
	);

	for (int i = 0; i < source->w; i++) {
		int s = destination_x + i;
		for (int j = 0; j < source->h; j++) {
			int t = destination_y + j;
			int pixel_target = s + t * destination->w;
			int pixel_source = i + j * source->w;
			// printf("%d, %d, %d, %d\n", s, t, pixel_target, pixel_source);
			destination->data[pixel_target * 4 + 0] = source->data[pixel_source * 4 + 0];
			destination->data[pixel_target * 4 + 1] = source->data[pixel_source * 4 + 1];
			destination->data[pixel_target * 4 + 2] = source->data[pixel_source * 4 + 2];
			destination->data[pixel_target * 4 + 3] = source->data[pixel_source * 4 + 3];
		}
	}
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

extern "C" LEAN_EXPORT lean_obj_res lean_display_image(
	int32_t texture_id_box,
	float width,
	float height,
	float u0,
	float v0,
	float u1,
	float v1
) {
	// auto draw_list = ImGui::GetWindowDrawList();
	// draw_list->AddCallback(ImGui::GetPlatformIO().DrawCallback_SetSamplerNearest);
	ImGui::Image(
		(ImTextureID)(intptr_t)texture_id_box,
		// {0, 0},
		{(float)width, (float)height},
		{u0, v0},
		{u1, v1}
	);
	return lean_io_result_mk_ok(lean_box(0));
}

extern "C" LEAN_EXPORT lean_obj_res lean_BeginCombo(
	b_lean_obj_arg label,
	b_lean_obj_arg preview
) {
	auto c_label = lean_string_cstr(label);
	auto c_preview = lean_string_cstr(preview);
	auto result = ImGui::BeginCombo(
		c_label,
		c_preview
	);
	return lean_io_result_mk_ok(lean_box(result));
}

extern "C" LEAN_EXPORT lean_obj_res lean_Selectable(
	b_lean_obj_arg label,
	bool selected
) {
	auto c_label = lean_string_cstr(label);
	auto result = ImGui::Selectable(
		c_label,
		selected
	);
	return lean_io_result_mk_ok(lean_box(result));
}

extern "C" LEAN_EXPORT lean_obj_res lean_SetItemDefaultFocus(
) {
	ImGui::SetItemDefaultFocus();
	return lean_io_result_mk_ok(lean_box(0));
}

extern "C" LEAN_EXPORT lean_obj_res lean_EndCombo(
) {
	ImGui::EndCombo();
	return lean_io_result_mk_ok(lean_box(0));
}
