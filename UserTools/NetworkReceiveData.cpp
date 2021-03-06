#include "NetworkReceiveData.h"

NetworkReceiveData::NetworkReceiveData():Tool(){}


bool NetworkReceiveData::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;

  m_variables.Get("port",m_port);
  m_variables.Get("address",m_address);


  Receive=new zmq::socket_t(*(m_data->context), ZMQ_PULL);
  int a=12000;
  Receive->setsockopt(ZMQ_SNDTIMEO, a);
  Receive->setsockopt(ZMQ_RCVTIMEO, a);
  

  std::stringstream tmp;
  tmp<<"tcp://"<<m_address<<":"<<m_port;

  Receive->connect(tmp.str().c_str());

  m_variables.Get("cards",cards);
  m_variables.Get("channels",channels);
  m_variables.Get("buffersize",buffersize);
  m_variables.Get("verbose",m_verbose);

  m_data->LastSync.resize(cards);
  m_data->SequenceID.resize(cards);
  m_data->StartTime.resize(cards);
  m_data->CardID.resize(cards);
  m_data->channels.resize(cards,channels);
  m_data->buffersize.resize(cards,buffersize);
  m_data->fullbuffsize.resize(cards,channels*buffersize);

  for (int i=0;i<cards;i++){
    m_data->PMTID.push_back(new int[channels]);

    //Data = new double*[channels];
    //for(int i = 0; i < channels; ++i)Data[i] = new double[buffersize];

    m_data->Data.push_back(new double[(channels*buffersize)]);
  }

  m_data->trigdata= new TriggerData();

  m_data->InfoTitle="NetworkReceiveDataVariables";
  m_variables>>m_data->InfoMessage;
  m_data->GetTTree("RunInformation")->Fill();


  return true;
}


bool NetworkReceiveData::Execute(){
  //std::cout<< "in in execute"<<std::endl;

  if(m_data->Restart==1)Finalise();
  else if(m_data->Restart==2)Initialise("",*m_data);
  else{ 

    //std::cout<<"before trigger"<<std::endl;
  if(m_data->triggered){
    //std::cout<<"triggered"<<std::endl;
    //boost::progress_timer t;
    
    for (int i=0;i< m_data->carddata.size();i++){
      delete m_data->carddata.at(i);
    }
    m_data->carddata.clear();
    
    
    
    //new
    
    
    zmq::message_t message;
    
    //std::cout<<"adout to receive"<<std::endl;    
    if(Receive->recv(&message)){
      //std::cout<<"received"<<std::endl;
      std::istringstream iss(static_cast<char*>(message.data()));
      int cards;
      iss>>cards;
      
      //std::cout<<" d1 "<<cards<<std::endl;
      
      for(int i=0;i<cards;i++){
	//std::cout<<" d2"<<std::endl;
	
	CardData *tmp=new CardData;
	//std::cout<<" d3 "<<i<<std::endl;
	
	if(tmp->Receive(Receive)){
	  //std::cout<<" d4 "<<i<<std::endl;
	  
	  m_data->carddata.push_back(tmp);
	  //std::cout<<" d5"<<std::endl;
	}
	else{
	  Log("ERROR!! CardData Receive fail",0,m_verbose);
	  return false;
	}
      }
      //std::cout<<" d6"<<std::endl;
      
    }
    else{
      Log("ERROR!! Network data receive connection fail/timeout",0,m_verbose);
      return false;                              
      
    }
    
    //std::cout<<"making trigdata"<<std::endl;
    //TriggerData * tmp2= new TriggerData(false);
    //std::cout<<"attempting to receive trig data"<<std::endl;
    if(m_data->trigdata->Receive(Receive)){
      //std::cout<<"event times print out"<<std::endl;
      for (int i=0;i<m_data->trigdata->EventSize;i++){
	//std::cout<<m_data->trigdata->EventTimes[i]<<std::endl;
      }
      //std::cout<<"received trig data"<<std::endl;
      //     m_data->trigdata=tmp2;
      //std::cout<<"trig data coppied to datamodel"<<std::endl;
    }
    else{
      Log("ERROR!! Network trig data receive connection fail/timeout",0,m_verbose);
    }
    //std::cout<<"alldone with execute"<<std::endl;
    /*
      zmq::message_t message;
      Receive->recv(&message);
      std::cout<<*reinterpret_cast<int*>(message.data())<<std::endl;
      m_data->carddata.at(0)->LastSync=*(reinterpret_cast<int*>(message.data()));
    //std::istringstream iss(static_cast<char*>(message.data()));
    //std::cout<<"memcopy"<<std::endl;
    //std::memcpy( (void*) &(m_data->carddata.at(0)->LastSync), message.data(), sizeof  m_data->carddata.at(0)->LastSync);
    */


    //    std::cout<<" int test "<< m_data->carddata.at(0)->LastSync<<std::endl;

    //////////////old boost
    /*
  
    int more;
    size_t more_size = sizeof (more);
    // Receive->getsockopt(ZMQ_RCVMORE, &more, &more_size);
    
    //  zmq::message_t message;
    //        std::cout<<"about to get message"<<std::endl;
    
    // while (true){
    //    if(Receive->recv(&message)){

    for (int i=0;i<17;i++){
      zmq::message_t message;
      Receive->recv(&message);
      int size=*reinterpret_cast<int*>(message.data());
      std::cout<<size<<std::endl;
      //std::cout<<size<<std::endl;
      //std::cout<<message.data()<<" : "<< sizeof (message.data())<<std::endl;
      //std::string tmp;
      zmq::message_t message2;     
      Receive->recv(&message2);
      char* tmp=(static_cast<char*>(message2.data()));
      //    memcpy ((void *) &tmp, message.data() , tmp.length()+1);
      std::cout<<"d1"<<std::endl;
      //std::string test(tmp, size);
      
      //std::cout<<test<<std::endl;
 //      std::stringstream tmp;
      // tmp.rdbuf(reinterpret_cast<std::streambuf*>(message.data()));
      //std::cout<<tmp.str()<<std::endl;

      //	std::cout<<*(reinterpret_cast<std::string*>(message.data()))<<std::endl;
     
      std::stringstream iss;
      std::cout<<"d1.5"<<std::endl; 
      ofstream myfile;
      myfile.open ("example.txt");
    
      for(int i=0;i<message2.size();i++){
      	iss<<tmp[i];
	myfile<<tmp[i];
	if(i<10)std::cout<<tmp[i];
	if(i>size-10)std::cout<<tmp[i];

      }
      
      myfile.close();

      std::cout<<std::endl;
      //std::cout<<std::endl<<"d2"<<iss.str()<<std::endl;

      std::cout<<message2.size()<<std::endl;
      std::cout<<iss.str().length()<<std::endl<<iss.str().max_size()<<std::endl;;
      

      std::string hope;
      hope=iss.str();
      std::istringstream iss2;
      //      	std::cout<<iss2.str()<<std::endl;
	//  std::ostringstream archive_stream;
      // std::cout<<"d3 "<<iss.str().length()<<std::endl;
      //      std::istringstream ggg;
      iss.seekg(0,std::ios::end);
      bool good =iss.good();
      bool bad=iss.bad();
      bool eof=iss.eof();
      bool fail=iss.fail();
      bool rdstate=iss.rdstate();

   std::cout<<"d3 "<< iss.str().length()<<std::endl;
   std::cout<<"good "<<good <<std::endl;
   std::cout<<"bad "<< bad<<std::endl;
   std::cout<<"eof "<< eof<<std::endl;
   std::cout<<"fail "<<fail<<std::endl;
   std::cout<<"rdstate "<<rdstate<<std::endl;
   
   boost::iostreams::basic_array_source<char> device(iss.str().data(), iss.str().size());
   boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(device);
   std::cout<<"d3"<<std::endl;

   boost::archive::binary_iarchive ia(s);
   //   ia >> obj;

   // boost::archive::binary_iarchive ia(iss);
	
	//  std::cout<<"run number before is "<<m_data->RunNumber<<std::endl;
	std::cout<<"d4"<<std::endl;      
	
	
	CardData *data = new CardData;
		ia >> *data;
	m_data->carddata.push_back(data);
	
	
	
	
	//      ia >> *m_data;
	
	//	std::cout<<"d2"<<std::endl;
	//std::cout<<"run number after is "<<m_data->RunNumber<<std::endl;
	//std::cout<<"received data 3 "<<m_data->carddata.at(0)->Data[3]<<std::endl;
	
	
	//Receive->getsockopt(ZMQ_RCVMORE, &more, &more_size);
	//}
    //else return false;
    // if(more==0) break;
   
    */
    //}
	





    //        std::cout<<"card data size "<<m_data->carddata.size()<<std::endl;



  }
  }
  return true;
}


bool NetworkReceiveData::Finalise(){

  
  delete Receive;
  Receive=0;

  for (int i=0;i<m_data->PMTID.size();i++){

    delete m_data->PMTID.at(i);

  }

  m_data->PMTID.clear();


  for (int i=0;i< m_data->Data.size();i++){

    delete m_data->Data.at(i);

  }

  m_data->Data.clear();

  //  delete m_data->trigdata;
  // m_data->trigdata=0;

  return true;
}
