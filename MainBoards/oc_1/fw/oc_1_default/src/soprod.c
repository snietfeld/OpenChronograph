
#include "soprod.h"

// Modulo that does not allow negative results
int mod(int x, int m) {
  int r;
  if ( m < 0 ) m = -m;
  r = x % m;
  return r<0 ? r+m : r;
}

void motor_init(motor_t* m)
{
  m->pole = 0;
  m->t_tick_start_ms = 0;
  m->state = IDLE;
}


// Blocks until tick is complete (~10ms)
void motor_tick_fwd(motor_t* m)
{
  // Alternate current direction through coil
  if (m->pole == 0) {
    *(m->port_F0) &= ~(m->mask_F0);   // Clear
    delay(SOPROD_TDRIVE_MS);
    *(m->port_F0) |= m->mask_F0;    // Set
      
    m->pole = 1;
  }
  else {
    *(m->port_F1) &= ~(m->mask_F1);   // Clear
    delay(SOPROD_TDRIVE_MS);  
    *(m->port_F1) |= m->mask_F1;    // Set
      
    m->pole = 0;
  }
  m->angle_meas_deg = mod( m->angle_meas_deg+1, SOPROD_TICKS_PER_REV);
}

// Blocks until tick is complete (~10ms)
void motor_tick_rev(motor_t* m)
{
  // Alternate current direction through coil
  if (m->pole == 0) {
    *(m->port_R0) &= ~(m->mask_R0);   // Clear
    delay(SOPROD_TDRIVE_MS); 
    *(m->port_R0) |= m->mask_R0;    // Set
    
    m->pole = 1;
  }
  else {
    *(m->port_R0) &= ~(m->mask_R1);   // Clear
    delay(SOPROD_TDRIVE_MS); 
    *(m->port_R0) |= m->mask_R1;    // Set
    
    m->pole = 0;
  }
  m->angle_meas_deg = mod( m->angle_meas_deg-1, SOPROD_TICKS_PER_REV);
}

void motor_set_cmd(motor_t* m, int16_t cmd)
{
  m->angle_cmd_deg = mod(cmd, SOPROD_TICKS_PER_REV);  // Clip to (0-359)
}

// Blocks until desired hand angle is reached
void motor_update_angle(motor_t* m)
{
  int count = 0;
  int ticks_fwd;  // Number of forward ticks required to reach the cmd
  
  while ( m->angle_meas_deg != m->angle_cmd_deg )
    {
      if ( count > SOPROD_TICKS_PER_REV )
	{
	  break;
	}

      // Calculate number of forward ticks required
      // If more than 180, do reverse tick
      if ( m->angle_cmd_deg > m->angle_meas_deg )
	ticks_fwd = m->angle_cmd_deg - m->angle_meas_deg;
      else
	ticks_fwd = (SOPROD_TICKS_PER_REV - m->angle_meas_deg) + m->angle_cmd_deg;

      if ( ticks_fwd <= 180 )
	motor_tick_fwd( m );
      else
	motor_tick_rev( m );

      //count++;
    }
}


// Non-blocking tick
void motor_update(motor_t* m, unsigned long t_ms)
{

  // If IDLE, check if we need to start ticking
  if ( m->state == IDLE ){
    // Check cmd versus angle, return if we don't need to do anything
    if ( m->angle_meas_deg == m->angle_cmd_deg )
      {
	return;
      }
    
    // Calculate number of forward ticks required
    int ticks_fwd;  // Number of forward ticks required to reach the cmd
    
    if ( m->angle_cmd_deg > m->angle_meas_deg )
      ticks_fwd = m->angle_cmd_deg - m->angle_meas_deg;
    else
      ticks_fwd = (SOPROD_TICKS_PER_REV - m->angle_meas_deg) + m->angle_cmd_deg;
    
    
    // Energize coils
    if ( ticks_fwd <= 180 )    // If more than 180, do reverse tick
      {
	// Need to tick forward
	
	// Alternate current direction through coil
	if (m->pole == 0) {
	  *(m->port_F0) &= ~(m->mask_F0);   // Clear
	}
	else {
	  *(m->port_F1) &= ~(m->mask_F1);   // Clear
	}
	
	m->state = DRIVING_FWD;
	m->t_tick_start_ms = t_ms;
      }
    else
      {
	// Need to tick backward
	
	// Alternate current direction through coil
	if (m->pole == 0) {
	  *(m->port_R0) &= ~(m->mask_R0);   // Clear
	}
	else {
	  *(m->port_R1) &= ~(m->mask_R1);   // Clear
	}
	
	m->state = DRIVING_REV;
	m->t_tick_start_ms = t_ms;
      }
    return;    // Done for now
  }

  
  // Not idle, either driving or waiting
  unsigned long ms_since_start = t_ms - m->t_tick_start_ms;

  // DRIVING ------------------------------
  if ( m->state == DRIVING_FWD ){
    // If currently ticking drive timeout reached, de-energize coils
    if ( ms_since_start >= SOPROD_TDRIVE_MS ){

      // Set all motor pins high to stop driving coils
      *(m->port_F0) |= m->mask_F0;
      *(m->port_F1) |= m->mask_F1;
      *(m->port_R0) |= m->mask_R0;
      *(m->port_R1) |= m->mask_R1;

      m->state = WAITING_FWD;    // Start waiting
    }
  }
  else if ( m->state == DRIVING_REV ){
    // If currently ticking drive timeout reached, de-energize coils
    if ( ms_since_start >= SOPROD_TDRIVE_MS ){

      // Set all motor pins high to stop driving coils
      *(m->port_F0) |= m->mask_F0;
      *(m->port_F1) |= m->mask_F1;
      *(m->port_R0) |= m->mask_R0;
      *(m->port_R1) |= m->mask_R1;

      m->state = WAITING_REV;    // Start waiting
    }
  }

  // WAITING ------------------------------
  if ( m->state == WAITING_FWD ){
    // If not done waiting, nothing to do
    if ( ms_since_start < (SOPROD_TDRIVE_MS + SOPROD_TWAIT_MS) ){
      return;
    }

    // If we get here, then we're done waiting
    // Toggle motor pole so next tick knows which direction to drive coils
    m->pole ^= 1;
    m->angle_meas_deg = mod( m->angle_meas_deg+1, SOPROD_TICKS_PER_REV);
    m->state = IDLE;
    m->t_tick_start_ms = 0;
  }
  
  else if ( m->state == WAITING_REV ){
    // If not done waiting, nothing to do
    if ( ms_since_start < (SOPROD_TDRIVE_MS + SOPROD_TWAIT_MS) ){
      return;
    }

    // If we get here, then we're done waiting
    // Toggle motor pole so next tick knows which direction to drive coils
    m->pole ^= 1;
    m->angle_meas_deg = mod( m->angle_meas_deg-1, SOPROD_TICKS_PER_REV);
    m->state = IDLE;
    m->t_tick_start_ms = 0;
  }

  return;
}
