#include <iostream>
#include <string.h>
#include <stdlib.h>     /* atoi */ 
using namespace std;
#include <netlink/socket.h>
#include <netlink/socket_group.h>

const unsigned SERVER_PORT = 5000;
char tab[10] = {0};
int aktualnyGracz = 1;

void sendInfo(NL::Socket* socket)
{
	socket->send(tab,10);
}

class OnAccept: public NL::SocketGroupCmd {
        void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {
            
            if (group->size() < 3)
            {
            	
            	string text = "Jestes graczem ";
                NL::Socket* newConnection = socket->accept();
                group->add(newConnection);
                cout << "\nConnection " << newConnection->hostTo() << ":" << newConnection->portTo() << " added...";
                cout << group->size();
                cout.flush();
                switch(group->size())
                {
                	case 2:
                		{
                			text += "X";
                			break;
                		}
                	case 3:
                		{
                			text += "O";
                			break;
                		}
                		default:
                			{
                				text += "error";
                				break;
                			}
                }
                
                newConnection->send(text.c_str(), text.size() + 1);
                if (group->size() == 3) {
                	sendInfo(group->get(1));
                }
            }
            else
            {
                
                cout <<"Nie ma miejsc" << endl;
                cout <<"Try again later :)" << endl;
                NL::Socket* newConnection = socket->accept();
                newConnection->disconnect();
                delete newConnection;
            }
            
            
        }
       
};


void wykonajRuch(int r)
{
	if ((r >= 1) && (r <= 9) && (tab[r] == 0))
		tab[r] = (aktualnyGracz == 1) ? 'X' : 'O';
	aktualnyGracz = (aktualnyGracz == 1) ? 2 : 1;
}

bool remis()
{
	for (int i = 1; i <= 9; i++)
		if (tab[i] == 0) return false;
	return true;
}

bool wygrana()
{
	bool test;
	char g = (aktualnyGracz == 2) ? 'X' : 'O';
	int i;

	test = false;
	for (i = 1; i <= 7; i += 3)
		test |= ((tab[i] == g) && (tab[i + 1] == g) && (tab[i + 2] == g));
	for (i = 1; i <= 3; i++)
		test |= ((tab[i] == g) && (tab[i + 3] == g) && (tab[i + 6] == g));
	test |= ((tab[1] == g) && (tab[5] == g) && (tab[9] == g));
	test |= ((tab[3] == g) && (tab[5] == g) && (tab[7] == g));

	return test;
}



class OnRead: public NL::SocketGroupCmd {
        void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {
                cout << "\nREAD -- ";
                cout.flush();
              	char buffer[256];
                buffer[255] = '\0';
                socket->read(buffer, 255);

                cout << "Message from " << socket->hostTo() << ":" << socket->portTo() << ". Text received: " << buffer;
                cout.flush();
                
                int ruch = atoi(buffer);
                wykonajRuch(ruch);
                if (wygrana() || remis()) {
                	for(unsigned i=1; i < (unsigned) group->size(); ++i) {
                		string msg = "GRA ZAKONCZONA: ";
                		msg += wygrana() ? "wygrales!" : "przegrales. :(";
                		group->get(i)->send(msg.c_str(), msg.size() + 1);
                	}
                }
                else {
                	sendInfo(group->get(aktualnyGracz));
                }
        }
};

class OnDisconnect: public NL::SocketGroupCmd {
        void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {
                group->remove(socket);
                cout << "\nClient " << socket->hostTo() << " disconnected...";
                cout.flush();
                delete socket;
        }
};

int main() {
        NL::init();
        cout << "\nStarting Server...";
        cout.flush();
        NL::Socket socketServer(SERVER_PORT);
        NL::SocketGroup group;
        OnAccept onAccept;
        OnRead onRead;
        OnDisconnect onDisconnect;
        group.setCmdOnAccept(&onAccept);
        group.setCmdOnRead(&onRead);
        group.setCmdOnDisconnect(&onDisconnect);
        group.add(&socketServer);
        while(true) {
                if(!group.listen(20000))
                        cout << "\nNo msg recieved during the last 20 seconds";
        }
        
}
