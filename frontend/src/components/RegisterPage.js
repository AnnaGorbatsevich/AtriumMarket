import React, { useState } from 'react';
import RoleModal from './RoleModal';
import RegistrationForm from './RegistrationForm';
import SuccessCard from './SuccessCard';

const RegisterPage = () => {
  const [role, setRole] = useState(null);
  const [result, setResult] = useState(null);

  if (result) {
    return (
      <SuccessCard
        result={result}
        onRestart={() => {
          setResult(null);
          setRole(null);
        }}
      />
    );
  }

  if (!role) {
    return <RoleModal onSelect={setRole} />;
  }

  return (
    <RegistrationForm role={role} onChangeRole={() => setRole(null)} onSubmitted={setResult} />
  );
};

export default RegisterPage;
