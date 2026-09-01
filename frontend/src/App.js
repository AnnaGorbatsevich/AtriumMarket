import React, { useState } from 'react';
import './App.css';
import RoleModal from './components/RoleModal';
import RegistrationForm from './components/RegistrationForm';
import SuccessCard from './components/SuccessCard';
import LoginForm from './components/LoginForm';

const App = () => {
  const [role, setRole] = useState(null);
  const [result, setResult] = useState(null);
  const [user, setUser] = useState(null);

  const handleRestart = () => {
    setResult(null);
    setRole(null);
    setUser(null);
  };

  if (user) {
    return <SuccessCard result={result} onRestart={handleRestart} />;
  }

  if (result) {
    return <LoginForm onSubmitted={setUser} />;
  }

  if (!role) {
    return <RoleModal onSelect={setRole} />;
  }

  return (
    <RegistrationForm role={role} onChangeRole={() => setRole(null)} onSubmitted={setResult} />
  );
};

export default App;
