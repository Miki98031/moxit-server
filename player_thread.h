#ifndef PLAYER_THREAD_H
#define PLAYER_THREAD_H

#include <iostream>

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

class PlayerThread {

public:

    void operator() (int clientSocket, int numOfConnectedPlayers);

    QJsonDocument getQjsonDocument();
};

#endif // PLAYER_THREAD_H
