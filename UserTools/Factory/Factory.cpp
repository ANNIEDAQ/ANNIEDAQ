#include "../Unity.cpp"
#include <Factory.h>

Tool* Factory(std::string tool){
Tool* ret=0;

// if (tool=="Type") tool=new Type;
//if (tool=="BoardReader") ret=new BoardReader;
//if (tool=="Monitoring") ret=new Monitoring;
//if (tool=="NetworkSendData") ret=new NetworkSendData;

if (tool=="VMETriggerSend")ret=new VMETriggerSend;

if (tool=="CrateReader") ret=new CrateReader;
if (tool=="VMESendData") ret=new VMESendData;
//if (tool=="DummyVMETriggerSend")ret=new DummyVMETriggerSend;



//if (tool=="CrateReaderDummy") ret=new CrateReaderDummy;
// if (tool=="Trigger") ret=new Trigger;
if (tool=="VMETriggerSendStream") ret=new VMETriggerSendStream;
if (tool=="CrateReaderStream") ret=new CrateReaderStream;
//if (tool=="VMESendData") ret=new VMESendData;
if (tool=="VMESendDataStream") ret=new VMESendDataStream;

  if (tool=="CrateReaderStream2") ret=new CrateReaderStream2;
return ret;
}

