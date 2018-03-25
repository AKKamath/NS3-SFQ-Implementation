/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 Universita' degli Studi di Napoli Federico II
 *               2016 University of Washington
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
 * Authors:  Stefano Avallone <stavallo@unina.it>
 *           Tom Henderson <tomhend@u.washington.edu>
 *           Pasquale Imputato <p.imputato@gmail.com>
 */

#ifndef IPV6_PACKET_FILTER_H
#define IPV6_PACKET_FILTER_H

#include "ns3/object.h"
#include "ns3/packet-filter.h"
#include "ns3/timer.h"
#include "ns3/event-id.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"

namespace ns3 {

/**
 * \ingroup ipv6
 * \ingroup traffic-control
 *
 * Ipv6PacketFilter is the abstract base class for filters defined for IPv6 packets.
 */
class Ipv6PacketFilter: public PacketFilter {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  Ipv6PacketFilter ();
  virtual ~Ipv6PacketFilter ();

private:
  virtual bool CheckProtocol (Ptr<QueueDiscItem> item) const;
  virtual int32_t DoClassify (Ptr<QueueDiscItem> item) const = 0;
};


/**
 * \ingroup internet
 *
 * FqCoDelIpv6PacketFilter is the filter to be added to the FQCoDel
 * queue disc to simulate the behavior of the fq-codel Linux queue disc.
 *
 */
class FqCoDelIpv6PacketFilter : public Ipv6PacketFilter {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  FqCoDelIpv6PacketFilter ();
  virtual ~FqCoDelIpv6PacketFilter ();

private:
  virtual int32_t DoClassify (Ptr<QueueDiscItem> item) const;

  uint32_t m_perturbation; //!< hash perturbation value
};

/**
 * \ingroup internet
 *
 * SfqIpv6PacketFilter is the filter to be added to the SFQ
 * queue disc to simulate the behavior of the sfq Linux queue disc.
 *
 */
class SfqIpv6PacketFilter : public Ipv6PacketFilter {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  SfqIpv6PacketFilter ();
  virtual ~SfqIpv6PacketFilter ();

private:
  virtual int32_t DoClassify (Ptr<QueueDiscItem> item) const;
  virtual void PerturbHash ();

  uint32_t m_perturbation;                //!< hash perturbation value
  Time m_perturbTime = MilliSeconds (100); //!< interval after which perturbation takes place
};

/**
 * \ingroup internet
 *
 * SfqNs2Ipv6PacketFilter is the filter to be added to the SFQ
 * queue disc to simulate the behavior of the sfq ns-2 queue disc.
 *
 */
class SfqNs2Ipv6PacketFilter : public Ipv6PacketFilter {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  SfqNs2Ipv6PacketFilter ();
  virtual ~SfqNs2Ipv6PacketFilter ();

private:
  virtual int32_t DoClassify (Ptr<QueueDiscItem> item) const;

};

} // namespace ns3

#endif /* IPV6_PACKET_FILTER */
