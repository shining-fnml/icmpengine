// the following header is required by the LGPL because
// we are using the actual time engine code
/*
 *   Copyright 2010 Shining <shining@freaknet.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
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
#include <pthread.h>
#include <unistd.h>
/*
#include <algorithm>
#include <QDate>
#include <QTime>
 
#include <KSystemTimeZones>
#include <KDateTime>
 
#include <Plasma/DataContainer>
*/
 
#include "icmpengine.h"


int Thread::Start(void * arg)
{
   Arg(arg); // store user data
   int code = pthread_create(&ThreadId, NULL, EntryPoint, this);
   return code;
}

void Thread::Run(void * arg)
{
	arg=NULL;
	Setup();
	Execute( Arg() );
}

/*static */
void *EntryPoint(void * pthis)
{
   Thread * pt = (Thread*)pthis;
   pt->Run( pt->Arg() );
   return NULL;
}

void cSource::Setup()
{
	oCommand = "ping -c 1 ";
	oCommand+=mTarget;
	oCommand.append(" >/dev/null 2>&1");
	Arg(strdup(oCommand.toLatin1()));
}

void cSource::Execute(void *target)
{
	FILE *fTarget;

	while (true) {
		if ((fTarget = popen((const char *)target, "w")) == NULL)
			currentStatus = unknown;
		else
			currentStatus = !pclose(fTarget);
		sleep (1);
	}
}
 
using namespace std;

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
	setMinimumPollingInterval(5000);
}

list <cSource *>::iterator IcmpEngine::findSource(const QString &name)
{
	list <cSource *>::iterator cSourceIterator;

	for (cSourceIterator=sources.begin(); cSourceIterator!=sources.end(); cSourceIterator++)
		if ((*cSourceIterator)->i_am(name))
			break;
	return cSourceIterator;
}
 
bool IcmpEngine::sourceRequestEvent(const QString &name)
{
	int result;
	cSource *pSource = new cSource(name);
	list <cSource *>::iterator cSourceIterator = findSource(name);

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
	return true;
}
 
bool IcmpEngine::updateSourceEvent(const QString &name)
{
	setData(name, I18N_NOOP(key), status[(*(findSource(name)))->getStatus()]);
	return true;
}
 
// This does the magic that allows Plasma to load
// this plugin.  The first argument must match
// the X-Plasma-EngineName in the .desktop file.
K_EXPORT_PLASMA_DATAENGINE(icmp, IcmpEngine)
 
// this is needed since IcmpEngine is a QObject
#include "icmpengine.moc"
