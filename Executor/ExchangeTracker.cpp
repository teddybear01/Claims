/*
 * ExchangeTracker.cpp
 *
 *  Created on: Aug 11, 2013
 *      Author: wangli
 */

#include "ExchangeTracker.h"
#include <string>
#include <sstream>
#include "../Logging.h"
#include "../Environment.h"
#include "../TimeOutReceiver.h"
ExchangeTracker::ExchangeTracker() {
	endpoint=Environment::getInstance()->getEndPoint();
	framework=new Theron::Framework(*endpoint);
//	framework->SetMaxThreads(2);
	logging_=new ExchangeTrackerLogging();
	std::ostringstream name;
	name<<"ExchangeTrackerActor://"+Environment::getInstance()->getIp();
	actor=new ExchangeTrackerActor(this,framework,name.str().c_str());
	logging_->log("ExchangeTrackerActor with name %s created!",name.str().c_str());


}

ExchangeTracker::~ExchangeTracker() {
	// TODO Auto-generated destructor stub
}
bool ExchangeTracker::RegisterExchange(unsigned long long int id, std::string port){
	if(id_to_port.find(id)!=id_to_port.end()){
		logging_->log("RegisterExchange fails because the exchange id has already existed.");
		return false;
	}
	id_to_port[id]=port;
	logging_->log("New exchange with id=%d (port %s)is successfully registered!",id,port.c_str());
	return true;
}
void ExchangeTracker::LogoutExchange(const unsigned long long int &id){
	id_to_port.erase(id_to_port.find(id));
	logging_->log("Exchange with id=%d is logged out!",id);
}

int ExchangeTracker::AskForSocketConnectionInfo(unsigned long long int exchange_id,std::string target_ip){
	TimeOutReceiver* receiver=new TimeOutReceiver(endpoint);
	Theron::Catcher<Message256> ResultCatcher;
	receiver->RegisterHandler(&ResultCatcher,&Theron::Catcher<Message256>::Push);
	bool send_result=framework->Send(exchange_id,receiver->GetAddress(),Theron::Address(("ExchangeTrackerActor://"+target_ip).c_str()));
	unsigned Timeout=3000;	//timeout in millisecond

	if(receiver->TimeOutWait(1,Timeout)==0){
		/**
		 * TODO: In current version, the request is only tried once. In the future,
		 * the request should be sent repeatedly until the reply is received or the
		 * times of timeouts exceeds some threshold.
		 */
		logging_->elog("Timeout when asking node[%s] for the connection info, the request Exchange ID is %u",target_ip.c_str(),exchange_id);
		return 0;
	}
//	receiver.Wait(1);

	Message256 feedback;
	Theron::Address from;
	ResultCatcher.Pop(feedback,from);
	NodeConnectionMessage NCM=NodeConnectionMessage::deserialize(feedback);
	logging_->log("Receive Socket connection info from <%s>, content: %s:%s",from.AsString(),NCM.ip.c_str(),NCM.port.c_str());
//	receiver.~Receiver();
	return atoi(NCM.port.c_str());
}

ExchangeTracker::ExchangeTrackerActor::ExchangeTrackerActor(ExchangeTracker* et,Theron::Framework* framework,const char* Name)
:et(et),Actor(*framework,Name){
	RegisterHandler(this,&ExchangeTracker::ExchangeTrackerActor::AskForConnectionInfo);
}

void ExchangeTracker::ExchangeTrackerActor::AskForConnectionInfo(const unsigned long long int &exchange_id, const Theron::Address from){
	et->logging_->log("%s is asking for the socket connecton info!",from.AsString());

	if(et->id_to_port.find(exchange_id)!=et->id_to_port.end()){

		NodeConnectionMessage myNCM(Environment::getInstance()->getIp(),et->id_to_port[exchange_id]);
		Send(NodeConnectionMessage::serialize(myNCM),from);
		et->logging_->log("The ask is answered!");
	}
	else{
		Send(NodeConnectionMessage::serialize(NodeConnectionMessage("0","0")),from);
		et->logging_->elog("No exchange matched for %lld!",exchange_id);
	}


}
