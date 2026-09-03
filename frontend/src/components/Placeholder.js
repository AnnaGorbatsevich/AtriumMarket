import React from 'react';

const Placeholder = ({ title }) => (
  <div className="container">
    <div className="card">
      <div className="header">
        <h1>{title}</h1>
      </div>
      <div className="card-body" style={{ padding: '2rem', textAlign: 'center' }}>
        <p className="success-subtext">Этот раздел ещё в разработке.</p>
      </div>
    </div>
  </div>
);

export default Placeholder;
