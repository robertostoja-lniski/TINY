#include <iostream>
#include <utility>

#include "P2PNode.h"

P2PNode::P2PNode(){}

ActionResult P2PNode::uploadFile(std::string uploadFileName) {

    File file(std::move(uploadFileName), name);
    // nie jest potrzebna tutaj synchronizacja,
    // poniewaz ten sam watek dodaje i usuwa pliki
    AddFileResult ret = localFiles.addFile(file);
    if(ret == ADD_ALREADY_EXISTS){
        return ACTION_NO_EFFECT;
    }

    if(ret == ADD_SUCCESS) {
        return ACTION_SUCCESS;
    }
}

ActionResult P2PNode::showLocalFiles() {
    localFiles.print();
}

ActionResult P2PNode::revoke(std::string revokeFileName) {
    File tmp(std::move(revokeFileName), name);
    localFiles.removeFile(tmp);
//    BroadcastFiles();
    return ACTION_SUCCESS;
}

ActionResult P2PNode::downloadFile(const std::string&) {
    return ACTION_NOT_HANDLED;
}

ActionResult P2PNode::updateLocalFiles(void) {
    return ACTION_NOT_HANDLED;
}

ActionResult P2PNode::showGlobalFiles(void) {
    return ACTION_NOT_HANDLED;
}

ActionResult P2PNode::BroadcastFilesFrequently(double period) {
    return ACTION_NOT_HANDLED;
}

ActionResult P2PNode::broadcastFiles() {
    return ACTION_NOT_HANDLED;
}

void P2PNode::setName() {
    // pozyskaj nazwę użytkownika z systemu UNIX
}

