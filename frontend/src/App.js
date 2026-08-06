import React, { useState } from 'react';
import './App.css';
import RoleModal from './components/RoleModal';
import RegistrationForm from './components/RegistrationForm';
import SuccessCard from './components/SuccessCard';

const App = () => {
  const [role, setRole] = useState(null);
  const [result, setResult] = useState(null);

  const handleRestart = () => {
    setResult(null);
    setRole(null);
  };

  if (result) {
    return <SuccessCard result={result} onRestart={handleRestart} />;
  }

  if (!role) {
    return <RoleModal onSelect={setRole} />;
  }

  return (
    <RegistrationForm role={role} onChangeRole={() => setRole(null)} onSubmitted={setResult} />
  );
};

export default App;
