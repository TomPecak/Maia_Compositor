// #include <QGuiApplication>
// #include <QCoreApplication>
// #include <QOpenGLContext>
// #include <QOffscreenSurface>
// #define GLFW_EXPOSE_NATIVE_GLX
// #include <GLFW/glfw3.h>
// #include <GLFW/glfw3native.h>
// #include <iostream>

// int main(int argc, char *argv[])
// {
//   QGuiApplication app(argc, argv);


//   if(!glfwInit()){
//     std::cerr << "Failed initialize glfw";
//     exit(1);
//   }

//   glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
//   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//   glfwWindowHint(GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_CORE_PROFILE);

//   GLFWwindow* window = glfwCreateWindow(800, 600, "Qml in GLFW window", nullptr, nullptr);
//   if(!window){
//     std::cout << "Wndow creation failed!";
//     exit(EXIT_FAILURE);
//   }else{
//     std::cout << "Sucessful window creation!" << std::endl;
//   }

//   glfwMakeContextCurrent(window);

//   GLXContext glxContext = glfwGetGLXContext(window);
//   QOpenGLContext *glfwQtContext = QNativeInterface::QGLXContext::fromNative(glxContext);

//   glfwQtContext->create();

//   if(!glfwQtContext){
//     std::cout << "[ERROR] Failded to create QOpenGLContext from glfw context !!" << std::endl;
//     exit(EXIT_FAILURE);
//   }else{
//     std::cout << "[SUCCESS] Successfull create QOpenGLContext from glfw context !!" << std::endl;
//   }

//   QOffscreenSurface surface;
//   glfwQtContext->makeCurrent(&surface);

//   return 0;
// }


#include <QGuiApplication>
#include <QCoreApplication>
#include <QOpenGLContext>
#include <QOffscreenSurface>

#define GLFW_EXPOSE_NATIVE_WAYLAND // Wymagane, żeby podpowiedzieć GLFW środowisko
#define GLFW_EXPOSE_NATIVE_EGL

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>

int main(int argc, char *argv[])
{
  // Wymuszamy Waylanda dla Qt
  qputenv("QT_QPA_PLATFORM","wayland");
  QGuiApplication app(argc, argv);

          // POPRAWKA: Wymuszamy Waylanda również dla GLFW (wymaga GLFW 3.3+)
  glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);

  if(!glfwInit()){
    std::cerr << "Failed initialize glfw\n";
    exit(1);
  }

          // POPRAWKA: Wymuszamy, by GLFW na 100% użyło EGL
  glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  // POPRAWKA: Zła flaga hintu w Twoim kodzie. Zmieniono ANY na PROFILE.
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(800, 600, "Qml in GLFW window", nullptr, nullptr);
  if(!window){
    std::cerr << "Window creation failed!\n";
    exit(EXIT_FAILURE);
  } else {
    std::cout << "Successful window creation!" << std::endl;
  }

  glfwMakeContextCurrent(window);

  EGLContext eglContext = glfwGetEGLContext(window);
  EGLDisplay eGLDisplay = glfwGetEGLDisplay();

  if (eglContext == EGL_NO_CONTEXT || eGLDisplay == EGL_NO_DISPLAY) {
    std::cerr << "[ERROR] GLFW failed to provide EGL Context/Display!\n";
    exit(EXIT_FAILURE);
  }

  QOpenGLContext *glfwQtContext = QNativeInterface::QEGLContext::fromNative(eglContext, eGLDisplay);

  if(!glfwQtContext){
    std::cerr << "[ERROR] Failed to create QOpenGLContext from glfw context !!" << std::endl;
    exit(EXIT_FAILURE);
  } else {
    // Teraz jest bezpiecznie to zawołać
    glfwQtContext->create();
    std::cout << "[SUCCESS] Successfully created QOpenGLContext from glfw context !!" << std::endl;
  }

  return 0;
}