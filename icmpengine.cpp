/*
 *   Copyright 2010 Shining <shining@freaknet.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 3 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
 
#include "icmpengine.h"
#include <KLocalizedString>
#include <QtNetwork/QHostInfo>

#include <arpa/inet.h>
#include <unistd.h>

#define MINIMUM_INTERVAL 5000 // 5 seconds
#define MAGIC_SIZE	102

Q_LOGGING_CATEGORY(ICMPENGINE, "icmpengine")


void mac_to_magic(unsigned char *buffer, QString &mac)
{
	unsigned char address[6];
	mac.replace( ":", "" );
	mac.replace( "-", "" );
	bool check;
	for (int position=0; position<6; position++) {
		QStringRef ref = QStringRef(&mac, position*2, 2);
		address[position] = ref.toUShort(&check, 16);
		buffer[position] = 0xFF;
	}
	for (int position=1; position<17; position++) {
		unsigned char *cursor = buffer+position*6;
		memcpy((void *)cursor, address, 6);
	}
}

/* Stolen from https://shadesfgray.wordpress.com/2010/12/17/wake-on-lan-how-to-tutorial/ */
static void wol(in_addr_t ip_addr, void *tosend)
{
	int udpSocket;
	struct sockaddr_in udpClient, udpServer;
	int broadcast = 1;
	ssize_t sent;

	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

	/** you need to set this so you can broadcast **/
	if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
		qDebug() << "error: setsockopt (SO_BROADCAST)";
		return;
	}
	udpClient.sin_family = AF_INET;
	udpClient.sin_addr.s_addr = INADDR_ANY;
	udpClient.sin_port = 0;

	bind(udpSocket, (struct sockaddr*)&udpClient, sizeof(udpClient));

	/** set server end point (the broadcast addres)**/
	udpServer.sin_family = AF_INET;
	udpServer.sin_addr.s_addr = htonl(ip_addr | 0xFF);
	udpServer.sin_port = htons(7);

	/** send the packet **/
	sent = sendto(udpSocket, tosend, sizeof(unsigned char) * MAGIC_SIZE, 0, (struct sockaddr*)&udpServer, sizeof(udpServer));
	if (sent != MAGIC_SIZE)
		qDebug() << "warning: sent " << sent << " bytes instead of " << MAGIC_SIZE;
}

int Thread::Start(void * arg)
{
   Arg(arg); // store user data
   int code = pthread_create(&ThreadId, NULL, EntryPoint, this);
   return code;
}

void Thread::Run()
{
	Setup();
	Execute( Arg() );
}

/*static */
void *EntryPoint(void * pthis)
{
   Thread * pt = (Thread*)pthis;
   pt->Run();
   return NULL;
}

void Source::Setup()
{
	oCommand = "ping -c 1 ";
	oCommand+=mTarget;
	oCommand.append(" >/dev/null 2>&1");
	Arg(strdup(oCommand.toLatin1()));
}

void Source::Execute(void *target)
{
	FILE *fTarget;

	while (true) {
		if ((fTarget = popen((const char *)target, "w")) == NULL)
			currentStatus = unknown;
		else {
			int retcode = WEXITSTATUS(pclose(fTarget));
#if 0
			if (retcode)
				qCDebug(ICMPENGINE) << "Execute(" << (const char*)target << "): " << retcode;
#endif
			currentStatus = retcode > 1 ? unknown : !retcode;
		}
		sleep (1);
	}
}

IcmpEngine::IcmpEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args)
{
	// We ignore any arguments - data engines do not have much use for them
	Q_UNUSED(args)

	status[unknown] = "Unknown";
	status[offline] = "Offline";
	status[online] = "Online";

	key = "status";
	statusLatest = statusPrevious = unknown;
	// This prevents applets from setting an unnecessarily high
	// update interval and using too much CPU.
	setMinimumPollingInterval(MINIMUM_INTERVAL);

	qCDebug(ICMPENGINE) << "IcmpEngine()";
}

IcmpEngine::~IcmpEngine()
{
	qCDebug(ICMPENGINE) << "~IcmpEngine()";
}

std::list <Source *>::iterator IcmpEngine::findSource(const QString &name)
{
	std::list <Source *>::iterator cSourceIterator;

	for (cSourceIterator=sources.begin(); cSourceIterator!=sources.end(); cSourceIterator++)
		if ((*cSourceIterator)->i_am(name))
			break;
	return cSourceIterator;
}
 
bool IcmpEngine::sourceRequestEvent(const QString &name)
{
	QStringList splitted = name.split("/");
	switch (splitted.size()) {
		case 1:
			pingRequestEvent(name);
			break;
		case 2:
			wake(splitted[0], splitted[1]);
			break;
		default:
			return false;
	}
	return true;
}

void IcmpEngine::pingRequestEvent(const QString &name)
{
	int result;
	Source *pSource = new Source(name);
	std::list <Source *>::iterator cSourceIterator = findSource(name);

	// qCDebug(ICMPENGINE) << "IcmpEngine::sourceRequestEvent";
	if (cSourceIterator != sources.end()) {
		delete (pSource);
		result = (*(cSourceIterator))->getStatus();
	}
	else {
		pSource->Start();
		sources.push_back(pSource);
		result = pSource->getStatus();
	}
	setData(name, I18N_NOOP(key), status[result]);
}

bool IcmpEngine::updateSourceEvent(const QString &source)
{
	// qCDebug(ICMPENGINE) << "IcmpEngine::updateSourceEvent(source =" << source << ")";
	setData(source, I18N_NOOP(key), status[(*(findSource(source)))->getStatus()]);
	return true;
}


void IcmpEngine::wake(QString host, QString mac)
{
	unsigned char tosend[MAGIC_SIZE];

	mac_to_magic(tosend, mac);
	QHostInfo info = QHostInfo::fromName(host);
	if (info.addresses().isEmpty()) {
		qCDebug(ICMPENGINE) << "cannot find host " << host;
		return;
	}
	QHostAddress address = info.addresses().first();
	wol(address.toIPv4Address(), tosend);
	// emit somePropertyChanged(1);
}

// This does the magic that allows Plasma to load
// this plugin.  The first argument must match
// the X-Plasma-EngineName in the .desktop file.
// K_EXPORT_PLASMA_DATAENGINE(icmp, IcmpEngine)
K_EXPORT_PLASMA_DATAENGINE_WITH_JSON(icmp, IcmpEngine, "plasma-engine-icmp.json")
 
// this is needed since IcmpEngine is a QObject
#include "icmpengine.moc"
