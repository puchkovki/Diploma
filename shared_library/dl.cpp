// Copyright [2020] <Puchkov Kyryll>
#include <dlfcn.h>
#include <string>
#include <iostream>

void invoke_method(const char *lib, const char *method) {
    /* Открываем совместно используемую библиотеку */
    void *dl_handle = dlopen(lib, RTLD_LAZY);
    if (dl_handle == NULL) {
        printf("!!! %s\n", dlerror());
        return;
    }

    /* Находим адрес функции в библиотеке */
    void(*func)() = (void(*) ()) dlsym(dl_handle, method);
    char* error = dlerror();
    if (error != NULL) {
        printf("!!! %s\n", error);
        return;
    }

    /* Вызываем функцию по найденному адресу */
    (*func)();

    /* Закрываем объект */
    dlclose(dl_handle);

    return;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cout << "Not enough arguments!" << std::endl;
        return 1;
    }

    // Имя библиотеки
    std::string libName = argv[1];
    // Имя функции
    std::string funcName = argv[2];
    invoke_method(libName.c_str(), funcName.c_str());

    return 0;
}
