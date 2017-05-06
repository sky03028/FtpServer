#include <iostream>
#include "ftpsession.h"
#include "service.h"
#include "ftpwarpper.h"
#include "utils.h"

int main()
{
    std::cout << "Welcome to my FtpServer......" << std::endl;

    Service *service = new Service;

    service->ServiceInit();

    service->ServiceStart();

    delete service;

    return 0;
}

