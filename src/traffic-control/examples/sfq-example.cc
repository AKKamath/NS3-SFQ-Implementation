/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 NITK Surathkal
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
 * Authors: Aditya Katapadi Kamath <akamath1997@gmail.com>
 *          A Tarun Karthik <tarunkarthik999@gmail.com>
 *          Anuj Revankar <anujrevankar@gmail.com>
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

 /** Network topology
 *
 *    10Mb/s, 2ms                            10Mb/s, 4ms
 * n0--------------|                    |---------------n4
 *                 |   1.5Mbps/s, 20ms  |
 *                 n2------------------n3
 *    10Mb/s, 3ms  |                    |    10Mb/s, 5ms
 * n1--------------|                    |---------------n5
 *
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/sfq-queue-disc.h"
#include "ns3/ipv4-packet-filter.h"
#include "ns3/ipv6-packet-filter.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SfqExample");

uint32_t checkTimes;
double avgQueueDiscSize;

// The times
double global_start_time;
double global_stop_time;
double sink_start_time;
double sink_stop_time;
double client_start_time;
double client_stop_time;

NodeContainer n0n2;
NodeContainer n1n2;
NodeContainer n2n3;
NodeContainer n3n4;
NodeContainer n3n5;

Ipv4InterfaceContainer i0i2;
Ipv4InterfaceContainer i1i2;
Ipv4InterfaceContainer i2i3;
Ipv4InterfaceContainer i3i4;
Ipv4InterfaceContainer i3i5;

std::stringstream filePlotQueueDisc;
std::stringstream filePlotQueueDiscAvg;

void
BuildAppsTest ()
{
  // SINK is in the right side
  uint16_t port = 50000;
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApp = sinkHelper.Install (n3n4.Get (1));
  sinkApp.Start (Seconds (sink_start_time));
  sinkApp.Stop (Seconds (sink_stop_time));

  // Connection one
  // Clients are in left side
  /*
   * Create the OnOff applications to send TCP to the server
   * onoffhelper is a client that send data to TCP destination
  */
  OnOffHelper clientHelper1 ("ns3::TcpSocketFactory", Address ());
  clientHelper1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelper1.SetAttribute ("PacketSize", UintegerValue (1000));
  clientHelper1.SetAttribute ("DataRate", DataRateValue (DataRate ("10Mb/s")));

  // Connection two
  OnOffHelper clientHelper2 ("ns3::TcpSocketFactory", Address ());
  clientHelper2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelper2.SetAttribute ("PacketSize", UintegerValue (1000));
  clientHelper2.SetAttribute ("DataRate", DataRateValue (DataRate ("10Mb/s")));

  ApplicationContainer clientApps1;
  AddressValue remoteAddress (InetSocketAddress (i3i4.GetAddress (1), port));
  clientHelper1.SetAttribute ("Remote", remoteAddress);
  clientApps1.Add (clientHelper1.Install (n0n2.Get (0)));
  clientApps1.Start (Seconds (client_start_time));
  clientApps1.Stop (Seconds (client_stop_time));

  ApplicationContainer clientApps2;
  clientHelper2.SetAttribute ("Remote", remoteAddress);
  clientApps2.Add (clientHelper2.Install (n1n2.Get (0)));
  clientApps2.Start (Seconds (client_start_time));
  clientApps2.Stop (Seconds (client_stop_time));
}

int
main (int argc, char *argv[])
{
  LogComponentEnable ("SfqQueueDisc", LOG_LEVEL_INFO);

  std::string sfqLinkDataRate = "1.5Mbps";
  std::string sfqLinkDelay = "20ms";

  std::string pathOut;
  bool writeForPlot = false;
  bool writePcap = false;
  bool flowMonitor = false;

  bool printSfqStats = true;

  global_start_time = 0.0;
  sink_start_time = global_start_time;
  client_start_time = global_start_time + 1.5;
  global_stop_time = 7.0;
  sink_stop_time = global_stop_time + 3.0;
  client_stop_time = global_stop_time - 2.0;

  // Configuration and command line parameter parsing
  // Will only save in the directory if enable opts below
  pathOut = "."; // Current directory
  CommandLine cmd;
  cmd.AddValue ("pathOut", "Path to save results from --writeForPlot/--writePcap/--writeFlowMonitor", pathOut);
  cmd.AddValue ("writeForPlot", "<0/1> to write results for plot (gnuplot)", writeForPlot);
  cmd.AddValue ("writePcap", "<0/1> to write results in pcapfile", writePcap);
  cmd.AddValue ("writeFlowMonitor", "<0/1> to enable Flow Monitor and write their results", flowMonitor);

  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create nodes");
  NodeContainer c;
  c.Create (6);
  Names::Add ( "N0", c.Get (0));
  Names::Add ( "N1", c.Get (1));
  Names::Add ( "N2", c.Get (2));
  Names::Add ( "N3", c.Get (3));
  Names::Add ( "N4", c.Get (4));
  Names::Add ( "N5", c.Get (5));
  n0n2 = NodeContainer (c.Get (0), c.Get (2));
  n1n2 = NodeContainer (c.Get (1), c.Get (2));
  n2n3 = NodeContainer (c.Get (2), c.Get (3));
  n3n4 = NodeContainer (c.Get (3), c.Get (4));
  n3n5 = NodeContainer (c.Get (3), c.Get (5));

  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
  // 42 = headers size
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1000 - 42));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (false));

  // SFQ params
  NS_LOG_INFO ("Set SFQ params");
  Config::SetDefault ("ns3::SfqQueueDisc::PacketLimit", UintegerValue (10 * 1024));
  Config::SetDefault ("ns3::SfqQueueDisc::Flows", UintegerValue (1024));

  NS_LOG_INFO ("Install internet stack on all nodes.");
  InternetStackHelper internet;
  internet.Install (c);

  TrafficControlHelper tchPfifo;
  uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
  tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxPackets", UintegerValue (10000));

  TrafficControlHelper tchSfq;
  tchSfq.SetRootQueueDisc ("ns3::SfqQueueDisc");
  tchSfq.AddPacketFilter(handle,"ns3::SfqIpv4PacketFilter");
  tchSfq.AddPacketFilter(handle,"ns3::SfqIpv6PacketFilter");

  NS_LOG_INFO ("Create channels");
  PointToPointHelper p2p;

  NetDeviceContainer devn0n2;
  NetDeviceContainer devn1n2;
  NetDeviceContainer devn2n3;
  NetDeviceContainer devn3n4;
  NetDeviceContainer devn3n5;

  QueueDiscContainer queueDiscs;

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  devn0n2 = p2p.Install (n0n2);
  tchPfifo.Install (devn0n2);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("3ms"));
  devn1n2 = p2p.Install (n1n2);
  tchPfifo.Install (devn1n2);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue (sfqLinkDataRate));
  p2p.SetChannelAttribute ("Delay", StringValue (sfqLinkDelay));
  devn2n3 = p2p.Install (n2n3);
  // only backbone link has SFQ queue disc
  queueDiscs = tchSfq.Install (devn2n3);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("4ms"));
  devn3n4 = p2p.Install (n3n4);
  tchPfifo.Install (devn3n4);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn3n5 = p2p.Install (n3n5);
  tchPfifo.Install (devn3n5);

  NS_LOG_INFO ("Assign IP Addresses");
  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  i0i2 = ipv4.Assign (devn0n2);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  i1i2 = ipv4.Assign (devn1n2);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  i2i3 = ipv4.Assign (devn2n3);

  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  i3i4 = ipv4.Assign (devn3n4);

  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  i3i5 = ipv4.Assign (devn3n5);

  // Set up the routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  BuildAppsTest ();

  if (writePcap)
    {
      PointToPointHelper ptp;
      std::stringstream stmp;
      stmp << pathOut << "/sfq";
      ptp.EnablePcapAll (stmp.str ().c_str ());
    }

  Ptr<FlowMonitor> flowmon;
  if (flowMonitor)
    {
      FlowMonitorHelper flowmonHelper;
      flowmon = flowmonHelper.InstallAll ();
    }

  if (writeForPlot)
    {
      filePlotQueueDisc << pathOut << "/" << "sfq-queue-disc.plotme";
      filePlotQueueDiscAvg << pathOut << "/" << "sfq-queue-disc_avg.plotme";

      remove (filePlotQueueDisc.str ().c_str ());
      remove (filePlotQueueDiscAvg.str ().c_str ());
      Ptr<QueueDisc> queue = queueDiscs.Get (0);
    }

  Simulator::Stop (Seconds (sink_stop_time));
  Simulator::Run ();

  QueueDisc::Stats st = queueDiscs.Get (0)->GetStats ();

  /*if (st.GetNDroppedPackets (SfqQueueDisc::FORCED_DROP) != 0)
    {
      std::cout << "There should be no drops due to queue full." << std::endl;
      exit (1);
    }*/

  if (flowMonitor)
    {
      std::stringstream stmp;
      stmp << pathOut << "/sfq.flowmon";

      flowmon->SerializeToXmlFile (stmp.str ().c_str (), false, false);
    }

  if (printSfqStats)
    {
      std::cout << "*** SFQ stats from Node 2 queue ***" << std::endl;
      //std::cout << "\t " << st.GetNDroppedPackets (SfqQueueDisc::UNFORCED_DROP)
        //        << " drops due to prob mark" << std::endl;
      //std::cout << "\t " << st.GetNDroppedPackets (SfqQueueDisc::FORCED_DROP)
        //        << " drops due to queue limits" << std::endl;
    }

  Simulator::Destroy ();

  return 0;
}
