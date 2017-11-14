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

#ifndef SFQ_QUEUE_DISC
#define SFQ_QUEUE_DISC

#include "ns3/queue-disc.h"
#include "ns3/object-factory.h"
#include <list>
#include <map>

namespace ns3 {

/**
 * \ingroup traffic-control
 *
 * \brief A flow queue used by the Sfq queue disc
 */

class sfqFlow : public QueueDiscClass {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief sfqFlow constructor
   */
  sfqFlow ();

  virtual ~sfqFlow ();

  /**
   * \enum FlowStatus
   * \brief Used to determine the status of this flow queue
   */
  enum FlowStatus
    {
      SFQ_EMPTY_SLOT,
      SFQ_IN_USE
    };

  /**
   * \brief Set the deficit for this flow
   * \param deficit the deficit for this flow
   */
  void SetAllot (uint32_t allot);
  /**
   * \brief Get the deficit for this flow
   * \return the deficit for this flow
   */
  int32_t GetAllot (void) const;
  /**
   * \brief Increase the deficit for this flow
   * \param deficit the amount by which the deficit is to be increased
   */
  void IncreaseAllot (int32_t allot);
  /**
   * \brief Set the status for this flow
   * \param status the status for this flow
   */
  void SetStatus (FlowStatus status);
  /**
   * \brief Get the status of this flow
   * \return the status of this flow
   */
  FlowStatus GetStatus (void) const;

private:
  int32_t m_allot;    //!< the deficit for this flow
  FlowStatus m_status;  //!< the status of this flow
};


/**
 * \ingroup traffic-control
 *
 * \brief An Sfq packet queue disc
 */

class SfqQueueDisc : public QueueDisc {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief SfqQueueDisc constructor
   */
  SfqQueueDisc ();

  virtual ~SfqQueueDisc ();

   /**
    * \brief Set the quantum value.
    *
    * \param quantum The number of bytes each queue gets to dequeue on each round of the scheduling algorithm
    */
   void SetQuantum (uint32_t quantum);

   /**
    * \brief Get the quantum value.
    *
    * \returns The number of bytes each queue gets to dequeue on each round of the scheduling algorithm
    */
   uint32_t GetQuantum (void) const;

private:
  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual Ptr<const QueueDiscItem> DoPeek (void) const;
  virtual bool CheckConfig (void);
  virtual void InitializeParams (void);

  /**
   * \brief Drop a packet from the head of the queue with the largest current byte count
   * \return the index of the queue with the largest current byte count
   */
  uint32_t SfqDrop (void);

  std::string m_interval;    //!< Interval attribute
  std::string m_target;      //!< Target attribute
  uint32_t m_limit;          //!< Maximum number of packets in the queue disc
  uint32_t m_quantum;        //!< Deficit assigned to flows at each round
  uint32_t m_flows;          //!< Number of flow queues
  uint32_t m_dropBatchSize;  //!< Max number of packets dropped from the fat flow

  uint32_t m_overlimitDroppedPackets; //!< Number of overlimit dropped packets

  std::list<Ptr<sfqFlow> > m_flowList;    //!< The list of new flows

  std::map<uint32_t, uint32_t> m_flowsIndices;    //!< Map with the index of class for each flow

  ObjectFactory m_flowFactory;         //!< Factory to create a new flow
  ObjectFactory m_queueDiscFactory;    //!< Factory to create a new queue
};

} // namespace ns3

#endif /* SFQ_QUEUE_DISC */
