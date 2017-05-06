#ifndef FTPSESSION_H
#define FTPSESSION_H

#include <iostream>
#include "session.h"

enum transfer_mode_e{
    PASV_MODE_ENABLE = 1,
    PORT_MODE_ENABLE = 2,
};


enum transfer_cmdtype_e{

    TRANSFER_PASV_STANDBY_REQ = 1,
    TRANSFER_PASV_STANDBY_RES,

    TRANSFER_PORT_STANDBY_REQ,
    TRANSFER_PORT_STANDBY_RES,

    TRANSFER_TRY_CONNNECT_REQ,
    TRANSFER_TRY_CONNNECT_RES,

    TRANSFER_SENDCOMMAND_REQ,
    TRANSFER_SENDCOMMAND_RES,

    TRANSFER_FILEUPLOAD_REQ,
    TRANSFER_FILEUPLOAD_RES,

    TRANSFER_FILEDOWNLOAD_REQ,
    TRANSFER_FILEDOWNLOAD_RES,

    TRANSFER_ABORT_REQ,
    TRANSFER_ABORT_RES,
};



class FtpSession : public Session
{
public:
    explicit FtpSession();
    explicit FtpSession(Session &s);

    FtpSession & operator() (Session &s)
    {
        mCtrlSockfd = s.getClientSocketID();
        mCtrlIpAddress = s.getClientIpAddress();
        mCtrlPort = s.getClientPort();

        return (*this);
    }

    FtpSession & operator= (Session &s)
    {
        mCtrlSockfd = s.getClientSocketID();
        mCtrlIpAddress = s.getClientIpAddress();
        mCtrlPort = s.getClientPort();

        return (*this);
    }

    int GetControlSocket();

    int GetTransferSocket();

    void SetControlSocket(int sockfd);

    void SetTransferSocket(int sockfd);

    unsigned int GetTransferIpAddress();

    void SetTransferIpAddress(unsigned int IpAddress);

    unsigned short GetTransferPort();

    void SetTransferPort(unsigned short port);


    int GetIPCContorlFD();

    int GetIPCTransferFD();

    void SetIPCControlFD(int ProcSock);

    void SetIPCTransferFD(int ProcSock);


    std::string GetCurrentPath();

    void SetCurrentPath(std::string Path);


    std::string GetRootPath();

    void SetRootPath(std::string Path);


    std::string GetPassword();

    void SetPassword(std::string &__password);


    std::string GetUsername();

    void SetUsername(std::string &__username);


    int GetTransferMode();

    void SetTransferMode(int __mode);


    int GetFtpDataPID();

    void SetFtpDataPID(int __pid);


    bool GetFtpDataAbort();

    void SetFtpDataAbortFlag(bool flag);


    int             PASVListenSock;
    unsigned short  PASVListenPort;

    int             PORTConnectSock;
    int             PORTConnectPort;

private:
    int             mTransmode;

    /* for communicate between control processer and transfer processer */
    int             IPC_FTPCTRLFD;
    int             IPC_FTPDATAFD;

    /* ftp-ctrl */
    int             mCtrlIpAddress;
    int             mCtrlPort;
    int             mCtrlSockfd;

    /* ftp-data */
    int             mTransIpAddress;
    int             mTransPort;
    int             mTransSockfd;

    int             mFtpDataPID;

    std::string     mUsername;
    std::string     mPassword;
    std::string     mDirPath;
    std::string     mRootPath;
    unsigned int    mSendInterval;

    bool        FtpDataAbortFlag;

};

#endif // FTPSESSION_H
