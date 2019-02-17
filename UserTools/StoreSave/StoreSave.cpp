#include "StoreSave.h"

StoreSave::StoreSave():Tool(){}


bool StoreSave::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;

  outstore=new BoostStore(false,0);

    m_variables.Get("verbose",m_verbose);  
  //m_variables.Get("VME_service_name",VME_service_name);
  m_variables.Get("numSources",numSources);
  m_variables.Get("OutPath",OutPath);
  m_variables.Get("OutFile",OutFile);
  //m_variables.Get("VME_port",VME_port);  
  //numTriggers=1;  
  part=0;
  std::stringstream tmp;
  tmp<<OutPath<<OutFile<<"p"<<part;
  Out=tmp.str();

  m_data->triggered=false;
  

  FindDataSources();
    

  m_data->MonitoringSocket=new zmq::socket_t(*m_data->context, ZMQ_PUB);
  m_data->MonitoringSocket->bind("tcp://*:63982");
  
  zmq::socket_t serviceadd (*m_data->context, ZMQ_PUSH);
  serviceadd.connect("inproc://ServicePublish");
  
  boost::uuids::uuid m_UUID;
  m_UUID = boost::uuids::random_generator()();
  std::stringstream test;
  test<<"Add "<< "MonitorData "<<m_UUID<<" 63982 "<<"0";
  zmq::message_t send(test.str().length());
  snprintf ((char *) send.data(), test.str().length() , "%s" ,test.str().c_str()) ;
  serviceadd.send(send);  
  

  
  //  m_data->InfoTitle="TriggerVariables";
  // m_variables>>m_data->InfoMessage;
  //m_data->GetTTree("RunInformation")->Fill();


  return true;
}


bool StoreSave::Execute(){

  FindDataSources();

  if(m_data->triggered){

    for (std::map<std::string,zmq::socket_t*>::iterator it=DataSources.begin(); it!=DataSources.end(); ++it){
     
      
      
      zmq::message_t out(2);
      it->second->send(out);

      zmq::message_t mesname;
      it->second->recv(&mesname);
      std::istringstream iss(static_cast<char*>(mesname.data()));
      std::string name=iss.str();

      zmq::message_t in;
      it->second->recv(&in);
      std::istringstream iss2(static_cast<char*>(in.data()));
      unsigned long arg2;
      iss2 >>  std::hex >> arg2;
      BoostStore* tmp;
      tmp=reinterpret_cast<BoostStore*>(arg2);
      std::cout<<"h1 "<<tmp<<std::endl;
      outstore->Set(name,tmp);
      std::cout<<"h2"<<std::endl;


    }

    outstore->Save(Out);

    zmq::message_t message(9);                  
    snprintf ((char *) message.data(), 9 , "%s" ,"DataFile") ;
    m_data->MonitoringSocket->send(message, ZMQ_SNDMORE);
    
    zmq::message_t message2(Out.length()+1);
    snprintf ((char *) message2.data(), Out.length()+1 , "%s" ,Out.c_str()) ; 
    m_data->MonitoringSocket->send(message2);


    std::stringstream tmp;
    part++;
    tmp<<OutPath<<OutFile<<"p"<<part;
    Out= tmp.str();
    //    delete outstore;
    //outstore=0;
    outstore= new BoostStore(false,0); 

  }


  return true;
}


bool StoreSave::Finalise(){


    for (std::map<std::string,zmq::socket_t*>::iterator it=DataSources.begin(); it!=DataSources.end(); ++it){


      zmq::message_t out(2);
      it->second->send(out);
      
      zmq::message_t mesname;
      it->second->recv(&mesname);
      std::istringstream iss(static_cast<char*>(mesname.data()));
      std::string name=iss.str();

      zmq::message_t in;
      it->second->recv(&in);
      std::istringstream iss2(static_cast<char*>(in.data()));
      unsigned long arg2;
      iss2 >>  std::hex >> arg2;
      BoostStore* tmp;
      tmp=reinterpret_cast<BoostStore *>(arg2);

      outstore->Set(name,tmp);

      delete it->second;
      it->second=0;
    }

    outstore->Save(Out);
    delete outstore;
    outstore=0;
    //    outstore= new BoostStore(false,0); 

    DataSources.clear();

    zmq::socket_t Ireceive (*m_data->context, ZMQ_PUSH);
    Ireceive.connect("inproc://ServicePublish");
    std::stringstream test;
    test<<"Delete "<< "MonitorData ";
    zmq::message_t send(test.str().length()+1);
    snprintf ((char *) send.data(), test.str().length()+1 , "%s" ,test.str().c_str()) ;
    Ireceive.send(send);


    delete m_data->MonitoringSocket;
    m_data->MonitoringSocket==0;


  return true;
}


void StoreSave::FindDataSources(){

  
  zmq::socket_t Ireceive (*(m_data->context), ZMQ_DEALER);
  Ireceive.connect("inproc://ServiceDiscovery");
  
  for(int i=0;i<11;i++){
  
  zmq::message_t send(8);
  snprintf ((char *) send.data(), 8 , "%s" ,"All NULL") ;
  
  Ireceive.send(send);
  
  zmq::message_t receive;
  Ireceive.recv(&receive);
  std::istringstream iss(static_cast<char*>(receive.data()));
  
  int size;
  iss>>size;
  
    
  for(int i=0;i<size;i++){
    
    Store *service = new Store;
    
    zmq::message_t servicem;
    Ireceive.recv(&servicem);
    
    std::istringstream ss(static_cast<char*>(servicem.data()));
    service->JsonParser(ss.str());
    
    std::string servicetype;
    std::string uuid;
    std::string ip;
    int port=0;
  
    service->Get("msg_value",servicetype);
    service->Get("uuid",uuid);
    service->Get("ip",ip);
    service->Get("remote_port",port); 
   //printf("%s \n",servicetype.c_str());
    if(servicetype=="DataSend" && DataSources.count(uuid)==0){
      zmq::socket_t *RemoteSend = new zmq::socket_t(*(m_data->context), ZMQ_DEALER);
      int a=12000;
      RemoteSend->setsockopt(ZMQ_SNDTIMEO, a);
      RemoteSend->setsockopt(ZMQ_RCVTIMEO, a);   
      
      std::stringstream tmp;
      tmp<<"tcp://"<<ip<<":"<<port;
      
      // printf("%s \n",tmp.str().c_str());
      RemoteSend->connect(tmp.str().c_str());
      DataSources[uuid]=RemoteSend;
    }

    else delete service;
    
  }
  
  
  }
}