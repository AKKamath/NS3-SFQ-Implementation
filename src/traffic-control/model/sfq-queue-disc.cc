/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 NITK Surathkal
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

#include "ns3/log.h"
#include "ns3/string.h"
#include "sfq-queue-disc.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SfqQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (SfqFlow);

TypeId SfqFlow::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SfqFlow")
    .SetParent<QueueDiscClass> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<SfqFlow> ()
  ;
  return tid;
}

SfqFlow::SfqFlow ()
  : m_deficit (0),
    m_status (INACTIVE)
{
  NS_LOG_FUNCTION (this);
}

SfqFlow::~SfqFlow ()
{
  NS_LOG_FUNCTION (this);
}

void
SfqFlow::SetDeficit (uint32_t deficit)
{
  NS_LOG_FUNCTION (this << deficit);
  m_deficit = deficit;
}

int32_t
SfqFlow::GetDeficit (void) const
{
  NS_LOG_FUNCTION (this);
  return m_deficit;
}

void
SfqFlow::IncreaseDeficit (int32_t deficit)
{
  NS_LOG_FUNCTION (this << deficit);
  m_deficit += deficit;
}

void
SfqFlow::SetStatus (FlowStatus status)
{
  NS_LOG_FUNCTION (this);
  m_status = status;
}

SfqFlow::FlowStatus
SfqFlow::GetStatus (void) const
{
  NS_LOG_FUNCTION (this);
  return m_status;
}


/*
int32_t hashfucntion(Ptr<QueueDiscItem> item)
{
  Address dest = Item.GetAddress();
  // seq is a number in the range 0-31
  return ((src >> seq)|(src << (32 - seq)))+dest;
}

*/


NS_OBJECT_ENSURE_REGISTERED (SfqQueueDisc);

TypeId SfqQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SfqQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<SfqQueueDisc> ()
    .AddAttribute ("Interval",
                   "The algorithm interval for each SFQ queue",
                   StringValue ("100ms"),
                   MakeStringAccessor (&SfqQueueDisc::m_interval),
                   MakeStringChecker ())
    .AddAttribute ("Target",
                   "The algorithm target queue delay for each SFQ queue",
                   StringValue ("5ms"),
                   MakeStringAccessor (&SfqQueueDisc::m_target),
                   MakeStringChecker ())
    .AddAttribute ("PacketLimit",
                   "The hard limit on the real queue size, measured in packets",
                   UintegerValue (10 * 1024),
                   MakeUintegerAccessor (&SfqQueueDisc::m_limit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Flows",
                   "The number of queues into which the incoming packets are classified",
                   UintegerValue (1024),
                   MakeUintegerAccessor (&SfqQueueDisc::m_flows),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("DropBatchSize",
                   "The maximum number of packets dropped from the fat flow",
                   UintegerValue (64),
                   MakeUintegerAccessor (&SfqQueueDisc::m_dropBatchSize),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

SfqQueueDisc::SfqQueueDisc ()
  : m_quantum (0),
    m_overlimitDroppedPackets (0)
{
  NS_LOG_FUNCTION (this);
}

SfqQueueDisc::~SfqQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

void
SfqQueueDisc::SetQuantum (uint32_t quantum)
{
  NS_LOG_FUNCTION (this << quantum);
  m_quantum = quantum;
}

uint32_t
SfqQueueDisc::GetQuantum (void) const
{
  return m_quantum;
}

bool
SfqQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);
  // ***add filters to the classify fucntion ***
  int32_t ret = Classify (item); //classify returns a hash function based on the packet filters

  if (ret == PacketFilter::PF_NO_MATCH)
    {
      NS_LOG_ERROR ("No filter has been able to classify this packet, drop it.");
      Drop (item);
      return false;
    }

  uint32_t h = ret % m_flows;

  Ptr<SfqFlow> flow;
  if (m_flowsIndices.find (h) == m_flowsIndices.end ())
    {
      NS_LOG_DEBUG ("Creating a new flow queue with index " << h);
      flow = m_flowFactory.Create<SfqFlow> ();
      Ptr<QueueDisc> qd = m_queueDiscFactory.Create<QueueDisc> ();
      qd->Initialize ();
      flow->SetQueueDisc (qd);
      AddQueueDiscClass (flow);

      m_flowsIndices[h] = GetNQueueDiscClasses () - 1;
    }
  else
    {
      flow = StaticCast<SfqFlow> (GetQueueDiscClass (m_flowsIndices[h]));
    }

/*  if (flow->GetStatus () == SfqFlow::INACTIVE)
    {
      flow->SetStatus (SfqFlow::NEW_FLOW);
      flow->SetDeficit (m_quantum);
      m_newFlows.push_back (flow);
    }
*/

  flow->GetQueueDisc ()->Enqueue (item);

  NS_LOG_DEBUG ("Packet enqueued into flow " << h << "; flow index " << m_flowsIndices[h]);

  if (GetNPackets () > m_limit)
    {
      SfqDrop ();
    }

  return true;
}
// TODO: Modify this
Ptr<QueueDiscItem>
SfqQueueDisc::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<SfqFlow> flow;
  Ptr<QueueDiscItem> item;

  do
    {
      bool found = false;

      while (!found && !m_newFlows.empty ())
        {
          flow = m_newFlows.front ();

          if (flow->GetDeficit () <= 0)
            {
              flow->IncreaseDeficit (m_quantum);
              flow->SetStatus (SfqFlow::OLD_FLOW);
              m_oldFlows.push_back (flow);
              m_newFlows.pop_front ();
            }
          else
            {
              NS_LOG_DEBUG ("Found a new flow with positive deficit");
              found = true;
            }
        }

      while (!found && !m_oldFlows.empty ())
        {
          flow = m_oldFlows.front ();

          if (flow->GetDeficit () <= 0)
            {
              flow->IncreaseDeficit (m_quantum);
              m_oldFlows.push_back (flow);
              m_oldFlows.pop_front ();
            }
          else
            {
              NS_LOG_DEBUG ("Found an old flow with positive deficit");
              found = true;
            }
        }

      if (!found)
        {
          NS_LOG_DEBUG ("No flow found to dequeue a packet");
          return 0;
        }

      item = flow->GetQueueDisc ()->Dequeue ();

      if (!item)
        {
          NS_LOG_DEBUG ("Could not get a packet from the selected flow queue");
          if (!m_newFlows.empty ())
            {
              flow->SetStatus (SfqFlow::OLD_FLOW);
              m_oldFlows.push_back (flow);
              m_newFlows.pop_front ();
            }
          else
            {
              flow->SetStatus (SfqFlow::INACTIVE);
              m_oldFlows.pop_front ();
            }
        }
      else
        {
          NS_LOG_DEBUG ("Dequeued packet " << item->GetPacket ());
        }
    } while (item == 0);

  flow->IncreaseDeficit (-item->GetPacketSize ());

  return item;
}

Ptr<const QueueDiscItem>
SfqQueueDisc::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);

  Ptr<SfqFlow> flow;

  if (!m_newFlows.empty ())
    {
      flow = m_newFlows.front ();
    }
  else
    {
      if (!m_oldFlows.empty ())
        {
          flow = m_oldFlows.front ();
        }
      else
        {
          return 0;
        }
    }

  return flow->GetQueueDisc ()->Peek ();
}

bool
SfqQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("SfqQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () == 0)
    {
      NS_LOG_ERROR ("SfqQueueDisc needs at least a packet filter");
      return false;
    }

  if (GetNInternalQueues () > 0)
    {
      NS_LOG_ERROR ("SfqQueueDisc cannot have internal queues");
      return false;
    }

  return true;
}

void
SfqQueueDisc::InitializeParams (void)
{
  NS_LOG_FUNCTION (this);

  // we are at initialization time. If the user has not set a quantum value,
  // set the quantum to the MTU of the device
  if (!m_quantum)
    {
      Ptr<NetDevice> device = GetNetDevice ();
      NS_ASSERT_MSG (device, "Device not set for the queue disc");
      m_quantum = device->GetMtu ();
      NS_LOG_DEBUG ("Setting the quantum to the MTU of the device: " << m_quantum);
    }

  m_flowFactory.SetTypeId ("ns3::SfqFlow");

  m_queueDiscFactory.SetTypeId ("ns3::QueueDisc");
  m_queueDiscFactory.Set ("Mode", EnumValue (Queue::QUEUE_MODE_PACKETS));
  m_queueDiscFactory.Set ("MaxPackets", UintegerValue (m_limit + 1));
  m_queueDiscFactory.Set ("Interval", StringValue (m_interval));
  m_queueDiscFactory.Set ("Target", StringValue (m_target));
}

uint32_t
SfqQueueDisc::SfqDrop (void)
{
  NS_LOG_FUNCTION (this);

  uint32_t maxBacklog = 0, index = 0;
  Ptr<QueueDisc> qd;

  /* Queue is full! Find the fat flow and drop packet(s) from it */
  for (uint32_t i = 0; i < GetNQueueDiscClasses (); i++)
    {
      qd = GetQueueDiscClass (i)->GetQueueDisc ();
      uint32_t bytes = qd->GetNBytes ();
      if (bytes > maxBacklog)
        {
          maxBacklog = bytes;
          index = i;
        }
    }

  /* Our goal is to drop half of this fat flow backlog */
  uint32_t len = 0, count = 0, threshold = maxBacklog >> 1;
  qd = GetQueueDiscClass (index)->GetQueueDisc ();
  Ptr<QueueItem> item;

  do
    {
      item = qd->GetInternalQueue (0)->Remove ();
      len += item->GetPacketSize ();
    } while (++count < m_dropBatchSize && len < threshold);

  m_overlimitDroppedPackets += count;

  return index;
}

} // namespace ns3
