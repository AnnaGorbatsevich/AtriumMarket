import React from 'react';

const Field = ({ label, error, children }) => (
  <div className="form-group">
    <label>{label}</label>
    {children}
    {error && <div className="field-error">{error}</div>}
  </div>
);

export default Field;
