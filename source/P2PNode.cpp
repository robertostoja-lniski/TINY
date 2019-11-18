#include <iostream>
#include "../include/P2PNode.h"

P2PNode::P2PNode(const std::string name) : name(name) {}

ActionResult P2PNode::uploadFile(std::string uploadFileName) {

    File file(uploadFileName, name);
    // nie jest potrzebna tutaj synchronizacja,
    // poniewaz ten sam watek dodaje i usuwa pliki
    localFiles.addFile(file);
    return ACTION_SUCCESS;
}

void P2PNode::showLocalFiles() {
    localFiles.print();
}

ActionResult P2PNode::revoke(std::string revokeFileName) {
    File tmp(revokeFileName, name);
    localFiles.removeFile(tmp);
//    BroadcastFiles();
    return ACTION_SUCCESS;
}
