#include <iostream>

#include "src/FtpService.h"
#include "src/service/FtpSession.h"
#include "src/service/FtpWarpper.h"
#include "src/utils/Utils.h"

int main() {
  std::cout << "Welcome to my FtpServer......" << std::endl;

  FtpService *service = new FtpService();

  service->Init();

  service->Start();

  delete service;

  return 0;
}

