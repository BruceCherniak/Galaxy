#pragma once

#include <pthread.h>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include "vector"
#include "sstream"
#include "debug.h"

#include "pvol.h"
#include "MessageManager.h"
#include "Work.h"
#include "KeyedObject.h"
#include "Events.h"

#include "threadpool11/threadpool11.hpp"
typedef threadpool11::Pool ThreadPool;

#include "rapidjson/document.h"

using namespace rapidjson;

#define APP_PRINT(x)																				\
{																														\
  stringstream ss;																					\
	ss x;																											\
	GetTheApplication()->Print(ss.str());											\
}

#define APP_LOG(x)																					\
{																														\
  stringstream ss;																					\
	ss << my_gettid() << " " << my_threadname() << " " <<  __FUNCTION__ << ": " x;			\
	GetTheApplication()->Log(ss.str());												\
}

struct ClassTableEntry
{
  ClassTableEntry(string s);
	ClassTableEntry(const ClassTableEntry&);
  ~ClassTableEntry();
  const char *c_str();
  string *my_string;                                   
  int indx;
};

class Application;
extern Application* GetTheApplication();

class Application {

public:
  Application();
  Application(int *argc, char ***argv);
  ~Application();

  MessageManager *GetTheMessageManager() { return theMessageManager; }
  KeyedObjectFactory *GetTheKeyedObjectFactory() { return theKeyedObjectFactory; }

  ThreadPool *GetTheThreadPool() { return threadPool; }

	void Print(std::string);
	void Log(std::string);

  void QuitApplication();
  void SyncApplication();

  int *GetPArgC() { return argcp; }
  char ***GetPArgV() { return argvp; }

  int GetSize() { return GetTheMessageManager()->GetSize(); }
  int GetRank() { return GetTheMessageManager()->GetRank(); }

  void Start(bool with_mpi = true);
  void Kill();
	void Wait();
  bool Running() { return !application_done; }

	void Pause() { GetTheMessageManager()->Pause(); }
	void Run() { GetTheMessageManager()->Run(); }

	void log(std::stringstream &s);

  Work *Deserialize(Message *msg);
  const char *Identify(Message *msg);

  int RegisterWork(string name, Work *(*f)(SharedP)) {
    int n = (*deserializers).size();
    (*deserializers).push_back(f);
		ClassTableEntry c(name.c_str());
		(*class_table).push_back(c);
    return n;
  }

  bool IsDoneSet() { return application_done; }
	pid_t get_pid() { return pid; }

	Document *OpenInputState(string s);
	Document *OpenOutputState();
	void SaveOutputState(Document *, string s);

private:
	MessageManager *theMessageManager;
	KeyedObjectFactory *theKeyedObjectFactory;

  vector<Work *(*)(SharedP)> *deserializers;
  vector<ClassTableEntry> *class_table;

  ThreadPool *threadPool;

  bool application_done;

  int *argcp;
  char ***argvp;

	pid_t pid;

  pthread_mutex_t lock;
  pthread_cond_t cond;

	EventTracker eventTracker;

private:
	class QuitMsg : public Work
	{
	public:
		QuitMsg(){};
		WORK_CLASS(QuitMsg, true)

	public:
		bool CollectiveAction(MPI_Comm coll_comm, bool isRoot);
	};

	class SyncMsg : public Work
	{
	public:
		SyncMsg(){};
		WORK_CLASS(SyncMsg, true)

	public:
		bool CollectiveAction(MPI_Comm coll_comm, bool isRoot);
	};

	class PrintMsg : public Work
	{
		WORK_CLASS(PrintMsg, false);
	public:
		PrintMsg(std::string &);
		bool Action(int sender);
	};
};

extern long my_gettid();
extern const char *threadname_by_id(long l);
extern const char *my_threadname();
extern void register_thread(std::string s);

