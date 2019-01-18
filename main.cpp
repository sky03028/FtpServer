#include <iostream>

#include "src/service/FtpService.h"
#include "src/utils/Utils.h"

int main() {
  std::cout << "Welcome to my FtpServer......" << std::endl;

  service::FtpService *service = new service::FtpService();

  service->Init();

  service->Start();

  delete service;

  return 0;
}

