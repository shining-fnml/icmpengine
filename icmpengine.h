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
 
// a standard include guard to prevent problems if the
// header is included multiple times
#ifndef ICMPENGINE_H
#define ICMPENGINE_H
 
#include <list>
// We need the DataEngine header, since we are inheriting it
#include <Plasma/DataEngine>

void * EntryPoint(void *);

class Thread
{
	public:
		Thread() {};
		virtual ~Thread() {};
		int Start(void * arg);
		void * Arg() const {return Arg_;}
		void Run(void * arg);
	protected:
		virtual void Setup() { /* Do any setup here */ };
		virtual void Execute(void*)=0; // to be overloaded
		void Arg(void* a){Arg_ = a;}
	private:
		pthread_t ThreadId;
		void * Arg_;

};

enum { offline, online, unknown };

class cSource : public Thread
{
	public:
		cSource (QString name) { mTarget=name; currentStatus=unknown; };
		bool isOnline() { return online; };
		// bool operator==(const QString &name) { return name==mTarget; };
		bool i_am(const QString &name) { return name==mTarget; };
		int getStatus() { return currentStatus; };
		int Start() { return Thread::Start(NULL);} ;
	protected:
		void Setup();
	private:
		QString mTarget, oCommand;
		bool online;
		void Execute(void*);
		int currentStatus;
};

/**
 * This engine checks if a specified host is online. It can deal with many
 * hosts at the same time.
 */
using namespace std;

class IcmpEngine : public Plasma::DataEngine
{
	// required since Plasma::DataEngine inherits QObject
	Q_OBJECT

	public:
		// every engine needs a constructor with these arguments
		IcmpEngine(QObject* parent, const QVariantList& args);

	protected:
		// this virtual function is called when a new source is requested
		bool sourceRequestEvent(const QString& name);

		// this virtual function is called when an automatic update
		// is triggered for an existing source (ie: when a valid update
		// interval is set when requesting a source)
		bool updateSourceEvent(const QString& source);
	private:
		const char *status[3];
		const char *key;
		int statusLatest, statusPrevious;
		list <cSource *> sources;
		list <cSource *>::iterator findSource(const QString &name);
};
 
#endif // ICMPENGINE_H
