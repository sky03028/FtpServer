#include "ftpsession.h"

FtpSession::FtpSession() :
    mTransSockfd(-1), mTransPort(0), PORTConnectPort(0), mTransIpAddress(0), mCtrlIpAddress(
        0), mCtrlSockfd(-1), mCtrlPort(0), PASVListenSock(-1), PASVListenPort(
        0), PORTConnectSock(-1), PORTConnectPort(0), mTransmode(0), FtpDataAbortFlag(false),
        IPC_FTPCTRLFD(0), IPC_FTPDATAFD(0), mSendInterval(0) {
  mDirPath = "/";
}

FtpSession::FtpSession(Session &s) {
  (*this)(s);
}

/* transfer socket */
void FtpSession::SetTransferSocket(int sockfd) {
  mTransSockfd = sockfd;
}

int FtpSession::GetTransferSocket() {
  return mTransSockfd;
}

/* control socket */
void FtpSession::SetControlSocket(int sockfd) {
  mCtrlSockfd = sockfd;
}

int FtpSession::GetControlSocket() {
  return mCtrlSockfd;
}

/* transfer ip address */
unsigned int FtpSession::GetTransferIpAddress() {
  return mTransIpAddress;
}

void FtpSession::SetTransferIpAddress(unsigned int IpAddress) {
  mTransIpAddress = IpAddress;
}

/* transfer port */
void FtpSession::SetTransferPort(unsigned short port) {
  mTransPort = port;
}

unsigned short FtpSession::GetTransferPort() {
  return mTransPort;
}

/* IPC socket */
int FtpSession::GetIPCContorlFD() {
  return IPC_FTPCTRLFD;
}

int FtpSession::GetIPCTransferFD() {
  return IPC_FTPDATAFD;
}

void FtpSession::SetIPCControlFD(int ProcSock) {
  IPC_FTPCTRLFD = ProcSock;
}

void FtpSession::SetIPCTransferFD(int ProcSock) {
  IPC_FTPDATAFD = ProcSock;
}

/* ftp path */
void FtpSession::SetCurrentPath(std::string Path) {
  mDirPath = Path;
}

std::string FtpSession::GetCurrentPath() {
  return mDirPath;
}

void FtpSession::SetRootPath(std::string Path) {
  mRootPath = Path;
}

std::string FtpSession::GetRootPath() {
  return mRootPath;
}

void FtpSession::SetUsername(std::string &__username) {
  mUsername = __username;
}

std::string FtpSession::GetUsername() {
  return mUsername;
}

void FtpSession::SetPassword(std::string &__password) {
  mPassword = __password;
}

std::string FtpSession::GetPassword() {
  return mPassword;
}

int FtpSession::GetTransferMode() {
  return mTransmode;
}

void FtpSession::SetTransferMode(int __mode) {
  mTransmode = __mode;
}

/* ftp-data pid */
int FtpSession::GetFtpDataPID() {
  return mFtpDataPID;
}

void FtpSession::SetFtpDataPID(int __pid) {
  mFtpDataPID = __pid;
}

bool FtpSession::GetFtpDataAbort() {
  return FtpDataAbortFlag;
}

void FtpSession::SetFtpDataAbortFlag(bool flag) {
  FtpDataAbortFlag = flag;
}

