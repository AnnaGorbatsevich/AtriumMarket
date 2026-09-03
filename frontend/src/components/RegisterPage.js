import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import RoleModal from './RoleModal';
import RegistrationForm from './RegistrationForm';
import SuccessCard from './SuccessCard';
import { useAuth } from '../auth';

const RegisterPage = () => {
  const [role, setRole] = useState(null);
  const [result, setResult] = useState(null);
  const { login } = useAuth();
  const navigate = useNavigate();

  if (result) {
    return (
      <SuccessCard
        result={result}
        onContinue={async () => {
          await login(result.token);
          navigate('/');
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
