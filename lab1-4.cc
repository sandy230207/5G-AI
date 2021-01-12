/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Manuel Requena <manuel.requena@cttc.es>
 */








#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include <iomanip>
#include "ns3/gnuplot.h"  //畫圖

using namespace ns3;
using namespace std;


NS_LOG_COMPONENT_DEFINE ("LenaX2HandoverMeasures");


int Handovernum= 0;
double TotalThroughput = 0;
//map<int, int> Handover_table; //抓取換手數量


void A3RsrpHandoverAlgorithm::DoReportUeMeas (uint16_t rnti, LteRrcSap::MeasResults measResults)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t) measResults.measId);

  if (measResults.measId != m_measId)
    {
      NS_LOG_WARN ("Ignoring measId " << (uint16_t) measResults.measId);
    }
  else 
    {
      if (measResults.haveMeasResultNeighCells
          && !measResults.measResultListEutra.empty ())
        {
          uint16_t bestNeighbourCellId = 0;
          uint8_t bestNeighbourRsrp = 0;

          for (std::list <LteRrcSap::MeasResultEutra>::iterator it = measResults.measResultListEutra.begin ();it != measResults.measResultListEutra.end ();++it)
            {
              if (it->haveRsrpResult)
                {
                  if ((bestNeighbourRsrp < it->rsrpResult)
                      && IsValidNeighbour (it->physCellId))
                    {
                      bestNeighbourCellId = it->physCellId;
                      bestNeighbourRsrp = it->rsrpResult;
                    }
                }
              else
                {
                  NS_LOG_WARN ("RSRP measurement is missing from cell ID " << it->physCellId);
                }
            }

          if (bestNeighbourCellId > 0)
            {
              NS_LOG_LOGIC ("Trigger Handover to cellId " << bestNeighbourCellId);
              NS_LOG_LOGIC ("target cell RSRP " << (uint16_t) bestNeighbourRsrp);
              NS_LOG_LOGIC ("serving cell RSRP " << (uint16_t) measResults.rsrpResult);

              // Inform eNodeB RRC about handover
              m_handoverManagementSapUser->TriggerHandover (rnti,
                                                            bestNeighbourCellId);
            }
        }
      else
        {
          NS_LOG_WARN (this << " Event A3 received without measurement results from neighbouring cells");
        }
    } // end of else of if (measResults.measId != m_measId)

} // end of DoReportUeMeasurment


void
NotifyConnectionEstablishedUe (string context,
                               uint64_t imsi,
                               uint16_t cellid,
                               uint16_t rnti)
{
  cout << context
            << " UE IMSI " << imsi
            << ": connected to CellId " << cellid
            << " with RNTI " << rnti
            << endl;
}

void
NotifyHandoverStartUe (string context,
                       uint64_t imsi,
                       uint16_t cellid,
                       uint16_t rnti,
                       uint16_t targetCellId)
{
  cout << context
            << " UE IMSI " << imsi
            << ": previously connected to CellId " << cellid
            << " with RNTI " << rnti
            << ", doing handover to CellId " << targetCellId
            << endl;
}
/*
void
NotifyHandoverEndOkUe (string context,
                       uint64_t imsi,
                       uint16_t cellid,
                       uint16_t rnti)
{
  Handovernum++;
  cout << context<< " UE IMSI " << imsi<< ": successful handover to CellId " << cellid<< " with RNTI " << rnti<< endl;

  
  
  cout<<"換手次數"<<Handovernum<<endl;
  cout<<"---------------------------------------------------------------------------"<<endl;
  ofstream outFile1;
  outFile1.open("HO-LAB1-4.txt",ofstream::out | ofstream::app);
  outFile1<<"Time   			: "<<int (Simulator::Now ().GetSeconds ())<<endl;
  outFile1 << context<< " UE IMSI " << imsi<< ": successful handover to CellId " << cellid<< " with RNTI " << rnti<< endl;
  outFile1<<"換手次數 : "<<Handovernum<<endl;
  outFile1<<"---------------------------------------------------------------------------"<<endl;
  int ttime = Simulator::Now ().GetSeconds ();
  Handover_table[ttime]++;

  outFile1.close();
}
*/
void
NotifyConnectionEstablishedEnb (string context,
                                uint64_t imsi,
                                uint16_t cellid,
                                uint16_t rnti)
{
  cout << context
            << " eNB CellId " << cellid
            << ": successful connection of UE with IMSI " << imsi
            << " RNTI " << rnti
            <<endl;
}

void
NotifyHandoverStartEnb (string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti,
                        uint16_t targetCellId)
{
  cout << context
            << " eNB CellId " << cellid
            << ": start handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << " to CellId " << targetCellId
            <<endl;
}

void
NotifyHandoverEndOkEnb (string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti)
{
  cout << context
            << " eNB CellId " << cellid
            << ": completed handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << endl;
}



void handler(NodeContainer Node)
{
   
   Vector position = Node.Get (0)->GetObject<MobilityModel> ()->GetPosition();

   cout<<endl<<"NodeUe 1 : x="<<position.x<<", y="<<position.y<<endl;
}

void ThroughputMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon,double xtime,Gnuplot2dDataset *dataset)
	{
		map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
		Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());
                ofstream outFile;
                outFile.open("Throughput-LAB1-4.txt",ofstream::out | ofstream::app);

		for (map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
		{
			Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
      cout<<"Time   			: "<<Simulator::Now ()/1000000000<<endl;
			cout<<"Flow ID			: " << stats->first <<" ; "<< fiveTuple.sourceAddress <<" -----> "<<fiveTuple.destinationAddress<<endl;

            
      cout<<"timeLastRxPacket = "<<fixed<<setprecision(10)<<stats->second.timeLastRxPacket.GetSeconds()<<endl;
      cout<<"timeFirstTxPacket = "<<stats->second.timeFirstTxPacket.GetSeconds()<<endl;
			cout<<"Duration		: "<<stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds()<<endl;
			cout<<"Last Received Packet	: "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds"<<endl;
			cout<<"Throughput: " << stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps"<<endl;
			
      cout<<"-------------------------------------------"<<endl;
      outFile<<"Time   			: "<<Simulator::Now ()/1000000000<<endl;
      outFile<<"Flow ID			: " << stats->first <<" ; "<< fiveTuple.sourceAddress <<" -----> "<<fiveTuple.destinationAddress<<endl;
      outFile<<"timeLastRxPacket = "<<fixed<<setprecision(10)<<stats->second.timeLastRxPacket.GetSeconds()<<endl;
      outFile<<"timeFirstTxPacket = "<<stats->second.timeFirstTxPacket.GetSeconds()<<endl;
      outFile<<"Duration		: "<<stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds()<<endl;
      outFile<<"Last Received Packet	: "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds"<<endl;
      outFile<<"Throughput: " << stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps"<<endl;
      
      outFile<<"---------------------------------------------------------------------------"<<endl;       

      double Throughput =stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())/1024/1024  ;
      dataset->Add((double)xtime ,(double) Throughput);
			xtime++;
      TotalThroughput=TotalThroughput + Throughput;

      cout<<"TotalThroughput: "<<TotalThroughput<<endl;
    }
    cout<<"################################################################################"<<endl;    
                
    outFile.close();

			Simulator::Schedule(Seconds(1),&ThroughputMonitor, fmhelper, flowMon,xtime,dataset);

	}

/**
 * Sample simulation script for an automatic X2-based handover based on the RSRQ measures.
 * It instantiates two eNodeB, attaches one UE to the 'source' eNB.
 * The UE moves between both eNBs, it reports measures to the serving eNB and
 * the 'source' (serving) eNB triggers the handover of the UE towards
 * the 'target' eNB when it considers it is a better eNB.
 */


int
main (int argc, char *argv[])
{
  
  cout<<endl<<"...Start Simulation..."<<endl<<endl;
  cout<<"...Set Parameters(Ue,EnB,Simulation Time)..."<<endl<<endl;
  uint16_t numberOfUes_fast = 15;
  uint16_t numberOfUes_slow = 60;
  

  uint16_t numberOfUes =numberOfUes_fast + numberOfUes_slow;

  uint16_t numberOfEnbs = 3; //original is 2
  uint16_t numberOffemtoEnbs = 15;
  uint16_t numBearersPerUe = 1;

  //uint16_t bandwidth = 20;
  //uint16_t bandwidth_ = 400; 
  double speed = 20;       // m/s
  double simTime = 30; //(double)(numberOfEnbs + 1) * distance / speed; // 1500 m / 20 m/s = 75 secs
  double enbTxPowerDbm = 46.0;
  double femtoTxPowerDbm =23.0;
  // change some default attributes so that they are reasonable for
  // this scenario, but do this before processing command line
  // arguments, so that the user is allowed to override these settings
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (320));//讓ue可以增加很多
  Config::SetDefault ("ns3::UdpClient::Interval", TimeValue (MilliSeconds (10)));
  Config::SetDefault ("ns3::UdpClient::MaxPackets", UintegerValue (1000000));
  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (true));
  Config::SetDefault("ns3::LteEnbMac::NumberOfRaPreambles", UintegerValue(10));

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue ("simTime", "Total duration of the simulation (in seconds)" , simTime);
  cmd.AddValue ("speed", "Speed of the UE (default = 20 m/s)", speed);
  cmd.AddValue ("enbTxPowerDbm", "TX power [dBm] used by HeNBs (defalut = 46.0)", enbTxPowerDbm);

  cmd.Parse (argc, argv);


  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
    //LteHelper *lteHelper = new LteHelper();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
    //PointToPointEpcHelper *epcHelper = new PointToPointEpcHelper();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
 
    
    lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisSpectrumPropagationLossModel"));
    lteHelper->SetHandoverAlgorithmType ("ns3::A3RsrpHandoverAlgorithm");
    lteHelper->SetHandoverAlgorithmAttribute ("Hysteresis",
                                              DoubleValue (3.0));
    lteHelper->SetHandoverAlgorithmAttribute ("TimeToTrigger",
                                              TimeValue (MilliSeconds (256)));

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);
  //ptr<FriisPropagationLossModel> lossmodel = CreateObject<FriisPropagationLossModel>();
  //lossmodel->SetLamda(2.40e9);

  // Routing of the Internet Host (towards the LTE network)
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  // interface 0 is localhost, 1 is the p2p device
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  NodeContainer ueNodes_fast;
  NodeContainer ueNodes_slow;
  NodeContainer MacroEnbs;
  NodeContainer femtoEnbs;

  MacroEnbs.Create (numberOfEnbs);
  femtoEnbs.Create (numberOffemtoEnbs);
  ueNodes_fast.Create (numberOfUes_fast);
  ueNodes_slow.Create (numberOfUes_slow);

  //femto mobility
  MobilityHelper femtoMobility;
  femtoMobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                 "X", StringValue ("ns3::UniformRandomVariable[Min=0|Max=1000]"),
                                 "Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max=1000]"));
  femtoMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  femtoMobility.Install (femtoEnbs);

  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (femtoTxPowerDbm));
  NetDeviceContainer femtoDevs = lteHelper->InstallEnbDevice (femtoEnbs);
  //lteHelper->SetEnbDeviceAttribute("DlBandwidth",UintegerValue(bandwidth));
  //lteHelper->SetEnbDeviceAttribute("UlBandwidth",UintegerValue(bandwidth));  
  //macro mobility

  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
   Vector enbPosition (500,250,0);
   enbPositionAlloc->Add (enbPosition);
   Vector enbPosition1 (250,750,0);
   enbPositionAlloc->Add (enbPosition1);
   Vector enbPosition2 (750,750,0);
   enbPositionAlloc->Add (enbPosition2);
  


  MobilityHelper macroMobility;
  /*
  macroMobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                 "X", StringValue ("ns3::UniformRandomVariable[Min=0|Max=800]"),
                                 "Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max=800]"));
  */
  macroMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  macroMobility.SetPositionAllocator (enbPositionAlloc);
  
  macroMobility.Install (MacroEnbs);

  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (enbTxPowerDbm));
  NetDeviceContainer macroDevs = lteHelper->InstallEnbDevice (MacroEnbs);
  //lteHelper->SetEnbDeviceAttribute("DlBandwidth",UintegerValue(bandwidth));
  //lteHelper->SetEnbDeviceAttribute("UlBandwidth",UintegerValue(bandwidth)); 

  NetDeviceContainer EnbDevs;
  EnbDevs.Add(macroDevs);
  EnbDevs.Add(femtoDevs);

  NodeContainer EnbNodes;
  EnbNodes.Add(MacroEnbs);
  EnbNodes.Add(femtoEnbs);

  //Ipv4AddressHelper address;
  //address.SetBase ("13.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer ipEnbs = ipv4h.Assign (EnbDevs);// new add

  //fast UE mobility
  MobilityHelper ueMobility;
  ueMobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                 "X", StringValue ("ns3::UniformRandomVariable[Min=0|Max=1000]"),
                                 "Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max=1000]"));
 
    
  ueMobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Mode", StringValue ("Time"),
                             "Time", StringValue ("2s"),
                             "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=40.0]"),
                             "Bounds", StringValue ("0|1000|0|1000"));
  ueMobility.Install (ueNodes_fast);
  NetDeviceContainer ueLteDevs_fast = lteHelper->InstallUeDevice (ueNodes_fast);

  //slow UE mobility
  MobilityHelper ueMobility1;
  ueMobility1.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                 "X", StringValue ("ns3::UniformRandomVariable[Min=0|Max=1000]"),
                                 "Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max=1000]"));
 
    
  ueMobility1.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Mode", StringValue ("Time"),
                             "Time", StringValue ("2s"),
                             "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=3.0]"),
                             "Bounds", StringValue ("0|1000|0|1000"));
  ueMobility1.Install (ueNodes_slow);
  NetDeviceContainer ueLteDevs_slow = lteHelper->InstallUeDevice (ueNodes_slow);


  NetDeviceContainer ueDevs;
  ueDevs.Add(ueLteDevs_slow);
  ueDevs.Add(ueLteDevs_fast);

  NodeContainer ueNodes;
  ueNodes.Add(ueNodes_slow);
  ueNodes.Add(ueNodes_fast);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIfaces;
  ueIpIfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));
  // Assign IP address to UEs, and install applications

  ofstream ipFile;
  ipFile.open("ueIP-LAB1-4.txt",ofstream::out | ofstream::app);
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);


      
      Ipv4Address ueIpAddr =  ueNodes.Get (u)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
      std::cout << "UE" << u << "'s Ipv4 Address " << ueIpAddr << std::endl;
      ipFile << "UE" << u << "'s Ipv4 Address " << ueIpAddr << std::endl;

    }
   ipFile.close();

  ofstream femtoipFile;
  femtoipFile.open("femtoIP-LAB1-4.txt",ofstream::out | ofstream::app);
  for (uint32_t u = 0; u < femtoEnbs.GetN (); ++u)  
    {
      Ipv4Address femtoIpAddr =  femtoEnbs.Get (u)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
      cout << "femtoIpAddr" << u << "'s Ipv4 Address " << femtoIpAddr << endl;
      femtoipFile<< "femtoIpAddr" << u << "'s Ipv4 Address " << femtoIpAddr << endl;
    }
  femtoipFile.close();

  ofstream MacroipFile;
  MacroipFile.open("MacroIP-LAB1-4.txt",ofstream::out | ofstream::app);
  for (uint32_t u = 0; u < MacroEnbs.GetN (); ++u)  
    {
      Ipv4Address MacroIpAddr =  MacroEnbs.Get (u)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
      cout << "MacroIpAddr" << u << "'s Ipv4 Address " << MacroIpAddr << endl;
      MacroipFile<< "MacroIpAddr" << u << "'s Ipv4 Address " << MacroIpAddr << endl;
    }
  MacroipFile.close();


  //lteHelper->Attach (ueDevs);
  lteHelper->AttachToClosestEnb (ueDevs, EnbDevs);


  NS_LOG_LOGIC ("setting up applications");
  
  // Install and start applications on UEs and remote host
  uint16_t dlPort = 10000;
  uint16_t ulPort = 20000;

  // randomize a bit start times to avoid simulation artifacts
  // (e.g., buffer overflows due to packet transmissions happening
  // exactly at the same time)
  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
  startTimeSeconds->SetAttribute ("Min", DoubleValue (0));
  startTimeSeconds->SetAttribute ("Max", DoubleValue (0.010));

  for (uint32_t u = 0; u < numberOfUes; ++u)
    {
      Ptr<Node> ue = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      for (uint32_t b = 0; b < numBearersPerUe; ++b)
        {
          ++dlPort;
          ++ulPort;

          ApplicationContainer clientApps;
          ApplicationContainer serverApps;

          NS_LOG_LOGIC ("installing UDP DL app for UE " << u);
          UdpClientHelper dlClientHelper (ueIpIfaces.GetAddress (u), dlPort);
          dlClientHelper.SetAttribute ("MaxPackets", UintegerValue (60000));
          dlClientHelper.SetAttribute ("Interval", TimeValue (Time ("0.01"))); //packets/s
          dlClientHelper.SetAttribute ("PacketSize", UintegerValue (1450));
          clientApps.Add (dlClientHelper.Install (remoteHost));
          PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory",
                                               InetSocketAddress (Ipv4Address::GetAny (), dlPort));
          serverApps.Add (dlPacketSinkHelper.Install (ue));

          NS_LOG_LOGIC ("installing UDP UL app for UE " << u);
          UdpClientHelper ulClientHelper (remoteHostAddr, ulPort);
          ulClientHelper.SetAttribute ("MaxPackets", UintegerValue (60000/2));
          ulClientHelper.SetAttribute ("Interval", TimeValue (Time ("0.01"))); //packets/s
          ulClientHelper.SetAttribute ("PacketSize", UintegerValue (1450));          
          clientApps.Add (ulClientHelper.Install (ue));
          PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory",
                                               InetSocketAddress (Ipv4Address::GetAny (), ulPort));
          serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

          Ptr<EpcTft> tft = Create<EpcTft> ();
          EpcTft::PacketFilter dlpf;
          dlpf.localPortStart = dlPort;
          dlpf.localPortEnd = dlPort;
          tft->Add (dlpf);
          EpcTft::PacketFilter ulpf;
          ulpf.remotePortStart = ulPort;
          ulpf.remotePortEnd = ulPort;
          tft->Add (ulpf);
          EpsBearer bearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
          lteHelper->ActivateDedicatedEpsBearer (ueDevs.Get (u), bearer, tft);

          Time startTime = Seconds (startTimeSeconds->GetValue ());
          serverApps.Start (startTime);
          clientApps.Start (startTime);

        } // end for b
    }


  // Add X2 inteface

  lteHelper->AddX2Interface (EnbNodes);//正確的

  // X2-based Handover
  //lteHelper->HandoverRequest (Seconds (0.100), ueLteDevs.Get (0), macroDevs.Get (0), macroDevs.Get (1));

  // Uncomment to enable PCAP tracing
  // p2ph.EnablePcapAll("lena-x2-handover-measures");

  lteHelper->EnablePhyTraces ();
  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();
  lteHelper->EnablePdcpTraces ();
  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats ();
  rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (1.0)));
  Ptr<RadioBearerStatsCalculator> pdcpStats = lteHelper->GetPdcpStats ();
  pdcpStats->SetAttribute ("EpochDuration", TimeValue (Seconds (1.0)));

  //Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverEndOk",  MakeCallback (&NotifyHandoverEndOkUe));
 // Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk", MakeCallback (&NotifyHandoverEndOkEnb));

  // Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/RecvMeasurementReport",  MakeCallback (&NotifyMeasureMentReport));//new add
  
//std::ostringstream oss;
//oss << "/NodeList/*" << "/$ns3::MobilityModel/CourseChange";

//Config::Connect (oss.str(), MakeCallback(&CourseChange));



  AnimationInterface anim ("LAB1-4.xml");

  for (uint32_t i = 0; i < remoteHostContainer.GetN (); ++i)
    {
     
      //anim.UpdateNodeDescription (remoteHostContainer.Get (i), "remotehost"); // Optional
     
      anim.UpdateNodeColor (remoteHostContainer.Get (i), 0, 0, 0); // Optional
    }

  for (uint32_t i = 0; i < MacroEnbs.GetN (); ++i)
    {
      //Ptr<LteEnbRrc> enbRrc = macroDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetRrc (); 
      //enbRrc->SetCellId(i);
      uint32_t temp =  macroDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetCellId();
      cout<<"macro-id "<<temp<<endl;
      //cout<<"MacroEnbs number = "<<MacroEnbs.GetN ()<<endl;   // 
      //anim.UpdateNodeDescription (MacroEnbs.Get (i), "MacroEnbs"); // Optional
     //anim.UpdateNodeSize(MacroEnbs.Get (i), 2, 2 );
      anim.UpdateNodeColor (MacroEnbs.Get (i), 0, 255, 0); // Optional
    }
    for (uint32_t i = 0; i < femtoEnbs.GetN (); ++i)
    {
      //Ptr<LteEnbRrc> enbRrc = femtoDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetRrc (); 
      //enbRrc->SetCellId(i);
      uint32_t temp =  femtoDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetCellId();
      cout<<"gNB-id "<<temp<<endl;

      //cout<<"femtoEnbs number = "<<MacroEnbs.GetN ()<<endl;   // 
      //anim.UpdateNodeDescription (femtoEnbs.Get (i), "gNB"); // Optional
     //anim.UpdateNodeSize(MacroEnbs.Get (i), 2, 2 );
      anim.UpdateNodeColor (femtoEnbs.Get (i), 255, 0, 0); // Optional
    }
  for (uint32_t i = 0; i < ueNodes_slow.GetN (); ++i)
    {
      //cout<<"macroUes number = "<<ueNodes_slow.GetN ()<<endl;   // 
      //anim.UpdateNodeDescription (ueNodes_slow.Get (i), "SUe"); // Optional
      //anim.UpdateNodeSize(ueNodes_slow.Get (i), 2, 2 );
      anim.UpdateNodeColor (ueNodes_slow.Get (i), 187, 255, 255); // Optional
    }
  for (uint32_t i = 0; i < ueNodes_fast.GetN (); ++i)
    {
      //cout<<"macroUes number = "<<ueNodes_fast.GetN ()<<endl;   // 
      //anim.UpdateNodeDescription (ueNodes_fast.Get (i), "FUe"); // Optional
      //anim.UpdateNodeSize(ueNodes_fast.Get (i), 2, 2 );
      anim.UpdateNodeColor (ueNodes_fast.Get (i), 0, 0, 255); // Optional
    }

    
/*
for(int i = 0; i<100 ;i=i+5) 
{
 Simulator::Schedule(Seconds(i),&handler,ueNodes); //show position of node(0)
}
*/
  double xtime = 0;

	std::string fileName = "Time VS Throughput 15enb-9";
  std::string graphicsFileName = fileName + ".png";//圖片檔名
  std::string plotFileName = fileName + ".plt";    //Gnuplot控制檔名
  std::string plotTitle = "Time Vs Throughput";    //圖片標題
  std::string dataTitle = "Throughput";            //資料標題

  Gnuplot2dDataset dataset;
  dataset.SetTitle ("Throughput");
  dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS); //線型

  Gnuplot gnuplot (graphicsFileName);
  gnuplot.SetTitle(plotTitle);
  gnuplot.SetTerminal("png");//設定圖片格式
  gnuplot.SetLegend("Time(Sec)","Throughput(Mbps)");//設定座標標籤
  
  //plot.AppendExtra ("set xrange [-6:+6]");// Set the range for the x axis.

//*********  FlowMonitor  ********* 

	FlowMonitorHelper fmHelper;
	Ptr<FlowMonitor> allMon = fmHelper.InstallAll();
  //Ptr<FlowMonitor> allMon = fmHelper.Install(femtoEnbs);//15enb-4
  //Ptr<FlowMonitor> allMon = fmHelper.Install(ueNodes);//15enb-3
  //15enb-2   beer=0 導致ue吞吐量都是零
  //15enb-1  macro
	Simulator::Schedule(Seconds(3),&ThroughputMonitor,&fmHelper, allMon,xtime,&dataset);

  //testTriggerHO (ueNodes.Get (0), ueLteDevs.Get (0), henbNodes, henbLteDevs);//換手
//********************************* 

  Simulator::Stop (Seconds (simTime)); //設定模擬停止時間

  Simulator::Run ();

  gnuplot.AddDataset(dataset);//把資料物件新增到gnuplot物件
  std::ofstream plotFile (plotFileName.c_str()); // Open the plot file.
  gnuplot.GenerateOutput(plotFile); // Write the plot file.
  plotFile.close();
 
  ThroughputMonitor(&fmHelper, allMon,xtime,&dataset);
  cout<<endl<<"End of Simulation"<<endl;

/*
  map<int,int>::iterator it;
  for(it=Handover_table.begin();it!=Handover_table.end();it++)
  {
   cout<<(*it).first<<":"<<(*it).second<<endl;
  }
*/

  Simulator::Destroy ();
  return 0;

}
